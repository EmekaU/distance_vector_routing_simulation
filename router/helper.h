#ifndef DISTANCE_VECTOR_ROUTING_SIMUL_HELPER_H
#define DISTANCE_VECTOR_ROUTING_SIMUL_HELPER_H

#include <iostream>
void DEBUG_PRINT(const std::string &msg, const std::string &detail);
void DEBUG_PRINT(const std::string &msg, int detail);

static char delimiter = '|';
static char pair_delimiter = ':';
static char comma_delimiter = ',';
struct NeighborInfo {
    std::string port;
    int cost;
};

struct timeval getTime();
#endif //DISTANCE_VECTOR_ROUTING_SIMUL_HELPER_H