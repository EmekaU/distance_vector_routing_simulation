#ifndef DISTANCE_VECTOR_ROUTING_SIMUL_ROUTERMANAGER_H
#define DISTANCE_VECTOR_ROUTING_SIMUL_ROUTERMANAGER_H
#include <list>
#include <map>
#include <thread>
#include <fstream>

#include "../router/Router.h"

class RouterManager {
    using NeighborMap = std::map<std::string, NeighborInfo>;
    using Topology = std::map<std::string, NeighborMap>;
    Topology _topology;
    std::vector<Router*> _routers;
    std::vector<std::thread> _router_threads;
    std::map<std::string, std::string> _port_map;
public:
    void Setup();
    void Simulate();
    void Process(const std::string &line);
    void ParseTopology();
};

#endif //DISTANCE_VECTOR_ROUTING_SIMUL_ROUTERMANAGER_H