#include "Router.h"

void Router::InitRouter(const char* port){
    constexpr addrinfo hints = {};
    addrinfo* hostAddressInfo;
    const int status = getaddrinfo(nullptr, port, &hints, &hostAddressInfo);
    int host_socket = get_socket(status, hostAddressInfo, true);
    if(host_socket == -1) {
        DEBUG_PRINT("Router " + this->name,  "failed to initialize\n");
        return;
    }
    if (listen(host_socket, MAX_NEIGHBOURS) == -1) {
        DEBUG_PRINT("Router " + this->name,  "failed to listen\n");
    }
}

int Router::GetAddressInfo(const char* port, addrinfo* hints, addrinfo** addressInfo) {

    hints->ai_family = AF_UNSPEC; /* unspecified - both ipv4 and ipv6 accepted */
    hints->ai_socktype = SOCK_STREAM; /* tcp */
    hints->ai_flags = AI_PASSIVE; /* need to bind to sockFD, fill in ip for me */
    return getaddrinfo(nullptr, port, hints, addressInfo);
}

int get_socket(const int addrinfo_status, addrinfo* addr_info_list, const bool is_host){
    constexpr int reuse_socket = 1;
    int sockFD = -1;
    addrinfo* ref;

    if(addrinfo_status != 0){
        DEBUG_PRINT("getaddrinfo", gai_strerror(addrinfo_status));
    }

    /* Find first bindable address in linked list */
    for(ref = addr_info_list; ref != nullptr; ref = ref->ai_next){

        /* get sockFD via socket call with domain, type, and protocol*/
        sockFD = socket(ref->ai_family, ref->ai_socktype, ref->ai_protocol);
        DEBUG_PRINT("sockFD", sockFD);
        if(sockFD == -1){
            perror("invalid socket\n");
            continue;
        }

        if (is_host == true){
            if(bind(sockFD, ref->ai_addr, ref->ai_addrlen) == -1){
                /**
                 * Bind to associate port with the socket file descriptor, addr ref and length of ref in bytes
                 * kernel will associate sockfd with the port in the addrinfo struct
                 */
                close(sockFD); /* close file descriptor */
                perror("server: bind\n");
                continue;
            }

            if(setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR, &reuse_socket, sizeof reuse_socket) == -1){
                perror("server: setsockopt\n");
                continue;
            }
        }
        else{
            if(connect(sockFD, ref->ai_addr, ref->ai_addrlen) == -1){
                close(sockFD);
                perror("client: connect");
                continue;
            }
        }
        break;
    }

    if(ref == nullptr){ // only exit program if host fails to bind to socket
        sockFD = -1;
        if (is_host == true){
            DEBUG_PRINT("Router",  "failed to bind\n");
        }else {
            DEBUG_PRINT("Router",  "failed to connect to neighbour\n");
        }
    }

    freeaddrinfo(addr_info_list);
    return sockFD;
}