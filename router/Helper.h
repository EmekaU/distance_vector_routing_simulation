#ifndef DISTANCE_VECTOR_ROUTING_SIMUL_HELPER_H
#define DISTANCE_VECTOR_ROUTING_SIMUL_HELPER_H

#include <iostream>
void DEBUG_PRINT(const std::string &msg, const std::string &detail);
void DEBUG_PRINT(const std::string &msg, int detail);

static constexpr char delimiter = '|';
static constexpr char pair_delimiter = ':';
static constexpr char comma_delimiter = ',';
struct NeighborInfo {
    std::string port;
    int cost;
};
#include <mutex>
extern std::mutex cout_mutex;
#endif //DISTANCE_VECTOR_ROUTING_SIMUL_HELPER_H