#ifndef DISTANCE_VECTOR_ROUTING_SIMUL_ROUTER_H
#define DISTANCE_VECTOR_ROUTING_SIMUL_ROUTER_H
#include <string>
#include <map>
#include <netdb.h>
#include "Helper.h"

class Router {
public:
    bool is_online;
    Router(std::string port, std::string name, const std::map<std::string, NeighborInfo>& neighbours);
    void InitRouter();
    void Run();

private:
    std::string name;
    std::string address;
    std::string port;
    struct RouteEntry {
        int cost;
        std::string next_hop;
    };
    std::map<std::string, RouteEntry> routing_table;
    int host_socket = -1;
    fd_set current_sockets{}, readFds{};

    addrinfo *hostAddressInfo = nullptr;

    struct Connection {
        std::string port;
        int socket = -1;
    };

    std::map<std::string, Connection> neighbors;

    using RouteAdvertisement = struct {
        std::string name;
        std::map<std::string, RouteEntry> route_costs;
    };
    using RouterInfo = struct {
        std::string name;
        std::string port;
        int cost;
    };


    static int GetAddressInfo(const char *port, addrinfo *hints, addrinfo **addressInfo);
    static int GetSocket(int addrinfo_status, addrinfo *addr_info_list, bool isHost);

    [[nodiscard]] int AcceptNewConnection() const;

    void GreetNeighbours();
    void BroadcastRoutingTable();

    void PrintRoutingTable() const;

    int TryReceivePacket(int socket, bool isGreet);

    void UpdateRoutingTable(const RouteAdvertisement *route_ad);

    [[nodiscard]] long SendRouteAd(int socket) const;

    [[nodiscard]] long SendInfo(int socket, int cost) const;

    static std::string Serialize(const RouteAdvertisement &route_ad);

    static std::string Serialize(const RouterInfo &router_info);

    static RouterInfo DeserializeToRouterInfo(const char *data);

    static RouteAdvertisement DeserializeToRouteAd(const char *data);
};

#endif //DISTANCE_VECTOR_ROUTING_SIMUL_ROUTER_H
