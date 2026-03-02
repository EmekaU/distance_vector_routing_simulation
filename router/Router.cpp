#include "Router.h"

#include <ranges>
#include <sstream>
#include <utility>
#include "helper.h"
#include <unistd.h>
#include <sys/time.h>

static constexpr int BROADCAST_INTERVAL = 5;

Router::Router(std::string port, std::string name, const std::map<std::string, NeighborInfo>& neighbours) : is_online(false),
    current_sockets(),
    readFds() {
    this->port = std::move(port);
    this->name = std::move(name);
    for (const auto &[router_name, info]: neighbours) {
        Connection connection = {info.port, -1};
        this->neighbors.insert_or_assign(router_name, connection);
        this->routing_table.insert_or_assign(router_name, info.cost);
    }
}

void Router::GreetNeighbours() {
    for (auto &[neighborName, neighborConn]: neighbors) {
        if (name >= neighborName) continue;

        addrinfo hints = {};
        addrinfo *neighborAddrInfo = nullptr;
        const int status = GetAddressInfo(neighborConn.port.c_str(), &hints, &neighborAddrInfo);

        int sock;
        try {
            sock = GetSocket(status, neighborAddrInfo, false);
        } catch (const std::exception &e) {
            DEBUG_PRINT("Router " + name, "failed to connect to " + neighborName + ": " + e.what());
            continue;
        }

        if (sock == -1) {
            DEBUG_PRINT("Router " + name, "could not create socket for " + neighborName);
            continue;
        }

        if (SendInfo(sock, routing_table.at(neighborName)) > 0) {
            neighborConn.socket = sock;
            FD_SET(sock, &current_sockets);
        } else {
            DEBUG_PRINT("Router " + name, "failed to greet " + neighborName);
            close(sock);
        }
    }
}

void Router::BroadcastRoutingTable() {
    for (auto &[neighborName, neighborConn]: neighbors) {
        if (neighborConn.socket == -1) {
            DEBUG_PRINT("Router " + name, "neighbor " + neighborName + " not connected yet, skipping");
            continue;
        }

        if (SendRouteAd(neighborConn.socket) <= 0) {
            DEBUG_PRINT("Router " + name, "failed to send to " + neighborName);
            close(neighborConn.socket);
            FD_CLR(neighborConn.socket, &current_sockets);
            neighborConn.socket = -1;
        }
    }
}

void Router::PrintRoutingTable() const {
    std::cout << "=== Router " << name << " Routing Table ===" << std::endl;
    for (const auto &[dest, cost]: routing_table) {
        std::cout << "  " << dest << " : " << cost << std::endl;
    }
    std::cout << "================================" << std::endl;
}

void Router::Run() {
    FD_SET(host_socket, &current_sockets);
    GreetNeighbours();

    timeval last_broadcast{};
    gettimeofday(&last_broadcast, nullptr);

    while (this->is_online) {
        timeval timeout{1, 0};
        readFds = current_sockets;

        const int ready = select(FD_SETSIZE, &readFds, nullptr, nullptr, &timeout);
        if (ready == -1) {
            if (errno == EINTR) continue;
            perror("An error occurred when checking for ready sockets");
            break;
        }

        // Check if it's time to broadcast
        timeval now{};
        gettimeofday(&now, nullptr);
        if ((now.tv_sec - last_broadcast.tv_sec) >= BROADCAST_INTERVAL) {
            BroadcastRoutingTable();
            PrintRoutingTable();
            last_broadcast = now;
        }

        if (ready == 0) continue;

        for (int socket = 0; socket < FD_SETSIZE; ++socket) {
            if (!FD_ISSET(socket, &readFds)) continue;

            if (socket == host_socket) {
                const int connection_socket = AcceptNewConnection();
                FD_SET(connection_socket, &current_sockets);
                if (TryReceivePacket(connection_socket, true) <= 0) {
                    FD_CLR(connection_socket, &current_sockets);
                    close(connection_socket);
                    DEBUG_PRINT("Router " + name, "dumped connection: " + std::to_string(connection_socket));
                }
            } else {
                if (TryReceivePacket(socket, false) <= 0) {
                    FD_CLR(socket, &current_sockets);
                    close(socket);
                    // Find and reset the neighbor's socket
                    for (auto &[neighborName, neighborConn]: neighbors) {
                        if (neighborConn.socket == socket) {
                            neighborConn.socket = -1;
                            break;
                        }
                    }
                }
            }
        }
    }
}

void Router::InitRouter() {
    addrinfo hints = {};
    const int status = GetAddressInfo(port.c_str(), &hints, &hostAddressInfo);
    host_socket = GetSocket(status, hostAddressInfo, true);
    hostAddressInfo = nullptr;
    if (host_socket == -1) {
        DEBUG_PRINT("Router " + this->name, "failed to initialize\n");
        return;
    }
    if (listen(host_socket, 4) == -1) {
        DEBUG_PRINT("Router " + this->name, "failed to listen\n");
        return;
    }
    this->is_online = true;
}

int Router::GetAddressInfo(const char *port, addrinfo *hints, addrinfo **addressInfo) {
    hints->ai_family = AF_UNSPEC;
    hints->ai_socktype = SOCK_STREAM;
    hints->ai_flags = AI_PASSIVE;
    return getaddrinfo(nullptr, port, hints, addressInfo);
}

int Router::GetSocket(const int addrinfo_status, addrinfo *addr_info_list, bool is_host) {
    constexpr int reuse_socket = 1;

    if (addrinfo_status != 0) {
        DEBUG_PRINT("getaddrinfo", gai_strerror(addrinfo_status));
        freeaddrinfo(addr_info_list);
        return -1;
    }

    // ReSharper disable once CppLocalVariableMayBeConst
    for (addrinfo *ref = addr_info_list; ref != nullptr; ref = ref->ai_next) {
        const int sockFD = socket(ref->ai_family, ref->ai_socktype, ref->ai_protocol);
        if (sockFD == -1) continue;

        if (is_host) {
            if (setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR, &reuse_socket, sizeof reuse_socket) == -1) {
                close(sockFD);
                continue;
            }
            if (bind(sockFD, ref->ai_addr, ref->ai_addrlen) == -1) {
                close(sockFD);
                continue;
            }
        } else {
            if (connect(sockFD, ref->ai_addr, ref->ai_addrlen) == -1) {
                close(sockFD);
                continue;
            }
        }

        freeaddrinfo(addr_info_list);
        return sockFD;
    }

    freeaddrinfo(addr_info_list);

    if (is_host) {
        throw std::runtime_error("failed to bind");
    }

    return -1;
}

int Router::AcceptNewConnection() const {
    sockaddr_storage incoming_addr{};
    uint addr_length = sizeof incoming_addr;
    const int conn_socket = accept(host_socket, reinterpret_cast<sockaddr *>(&incoming_addr), &addr_length);
    if (conn_socket == -1) {
        throw std::runtime_error("Failed to accept connection");
    }
    return conn_socket;
}

int Router::TryReceivePacket(const int socket, const bool isGreet) {
    char buffer[1024];
    const int numberOfBytes = recv(socket, buffer, sizeof(buffer) - 1, 0); // NOLINT(*-narrowing-conversions)

    if (numberOfBytes <= 0) {
        DEBUG_PRINT("Router " + name, "failed to receive from socket: " + std::to_string(socket));
        return numberOfBytes;
    }

    buffer[numberOfBytes] = '\0';

    if (isGreet) {
        const RouterInfo router_info = DeserializeToRouterInfo(buffer);
        Connection connection = {router_info.port, socket};
        neighbors.insert_or_assign(router_info.name, connection);
        routing_table.insert(std::pair(router_info.name, router_info.cost));
    } else {
        const RouteAdvertisement route_ad = DeserializeToRouteAd(buffer);
        UpdateRoutingTable(&route_ad);
    }

    return numberOfBytes;
}

void Router::UpdateRoutingTable(const RouteAdvertisement *route_ad) {
    const int cost_to_neighbour = routing_table[route_ad->name];
    for (const auto &[foreignRouterName, foreignRouterCost]: route_ad->route_costs) {
        int current_cost = std::numeric_limits<int>::max();
        if (routing_table.contains(foreignRouterName)) {
            current_cost = routing_table[foreignRouterName];
        }
        routing_table[foreignRouterName] = std::min(current_cost, cost_to_neighbour + foreignRouterCost);
    }
}

long Router::SendInfo(const int socket, const int cost) const {
    const RouterInfo router_info = {name, port, cost};
    const std::string data = Serialize(router_info);
    return send(socket, data.c_str(), data.size(), 0);
}

long Router::SendRouteAd(int socket) const {
    const RouteAdvertisement route_ad = {name, routing_table};
    const std::string data = Serialize(route_ad);
    return send(socket, data.c_str(), data.size(), 0);
}

std::string Router::Serialize(const RouteAdvertisement &route_ad) {
    std::string body = route_ad.name + delimiter;
    std::string route;
    for (const auto &[name, cost]: route_ad.route_costs) {
        route.append(name + pair_delimiter + std::to_string(cost) + comma_delimiter);
    }
    if (!route.empty()) {
        route.pop_back();
    }
    return body.append(route);
}

std::string Router::Serialize(const RouterInfo &router_info) {
    return router_info.name + pair_delimiter + std::to_string(router_info.cost) + delimiter + router_info.port;
}

Router::RouterInfo Router::DeserializeToRouterInfo(const char *data) {
    std::istringstream iss{std::string(data)};
    std::string name_cost, port_str;
    std::getline(iss, name_cost, delimiter);
    std::getline(iss, port_str, delimiter);

    std::istringstream pair_iss(name_cost);
    std::string name, cost_str;
    std::getline(pair_iss, name, pair_delimiter);
    std::getline(pair_iss, cost_str, pair_delimiter);

    return {name, port_str, std::stoi(cost_str)};
}

Router::RouteAdvertisement Router::DeserializeToRouteAd(const char *data) {
    std::istringstream iss{std::string(data)};
    std::string name, routes_str;
    std::getline(iss, name, delimiter);
    std::getline(iss, routes_str, delimiter);

    std::map<std::string, int> route_costs;
    std::istringstream routes_iss(routes_str);
    std::string entry;
    while (std::getline(routes_iss, entry, comma_delimiter)) {
        std::istringstream entry_iss(entry);
        std::string dest, cost_str;
        std::getline(entry_iss, dest, pair_delimiter);
        std::getline(entry_iss, cost_str, pair_delimiter);
        route_costs[dest] = std::stoi(cost_str);
    }

    return {name, route_costs};
}