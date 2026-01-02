#ifndef DISTANCE_VECTOR_ROUTING_SIMUL_HELPER_H
#define DISTANCE_VECTOR_ROUTING_SIMUL_HELPER_H

#include <iostream>
void DEBUG_PRINT(const std::string &msg, const std::string &detail);
void DEBUG_PRINT(const std::string &msg, int detail);

bool port_is_valid(char* port);
void verify(int val, char* msg);
long send_packet(int socket, void* packet, int packet_size);
struct timeval getTime();
int accept_new_connection(int server_socket);
int get_socket(int addrinfo_status, struct addrinfo* addr_info_list, bool is_host);
#endif //DISTANCE_VECTOR_ROUTING_SIMUL_HELPER_H