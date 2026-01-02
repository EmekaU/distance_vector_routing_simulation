//
// Created by Umeozo Emeka on 2026-01-01.
//

#ifndef DISTANCE_VECTOR_ROUTING_SIMUL_ROUTER_H
#define DISTANCE_VECTOR_ROUTING_SIMUL_ROUTER_H
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <errno.h>
#include "helper.h"
class Router {
public:
    std::string name;
    int id;
    std::string address;
    int port;

    void InitRouter(const char* port);
private:
    static int GetAddressInfo(const char* port, struct addrinfo* hints, struct addrinfo** addressInfo);
    void set_router_to_default(router* r);
    void print(Router* r);
    void printRoutingTable();
    int get_table_row_index(unsigned char c);
    bool is_router_name_valid(unsigned char c);
    void init_routing_table();
    void update_routing_table(unsigned char src, unsigned char dest, int cost);
    void run_router();
    void connect_to_neighbours();
    bool is_connected(int socket);
    long send_heartbeat(int socket);
    void print_next_hop_for_each_known_router();
    unsigned char get_next_hop(unsigned char router);
    long greet_neighbour(int socket);
};

#define MAX_NEIGHBOURS 25
typedef enum message_type{
    greet = 0, ack = 1, info = 2, heartbeat = 3
}message_type;

typedef struct message{
    message_type type;
    unsigned char dest;
    unsigned char src;
    int cost;
}message;

#endif //DISTANCE_VECTOR_ROUTING_SIMUL_ROUTER_H
