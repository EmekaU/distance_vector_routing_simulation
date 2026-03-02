#include "RouterManager.h"


#include "../router/Router.h"

void RouterManager::ParseTopology() {
    std::string line;
    std::ifstream f ("../config/initial_topology.txt");
    if (!f.is_open())
        throw std::runtime_error("error while opening file");
    while(getline(f, line)) {
        Process(line);
    }
    if (f.bad())
        throw std::runtime_error("error while opening file");
}

void RouterManager::Process(const std::string &line) {
    std::istringstream iss(line);
    std::string router_a, router_b;
    int cost;
    iss >> router_a >> router_b >> cost;
    _topology[router_a][router_b] = {_port_map[router_b], cost};
    _topology[router_b][router_a] = {_port_map[router_a], cost};
}

void RouterManager::Setup() {
    for (int i = 0; i <= 4; i++) {
        std::string name(1, static_cast<char>('A' + i));
        _port_map[name] = std::to_string(50001 + i);
    }

    ParseTopology();

    for (int i = 0; i <= 4; i++) {
        std::string name(1, static_cast<char>('A' + i));
        auto* router = new Router(_port_map[name], name, _topology[name]);
        router->InitRouter();
        _routers.push_back(router);
    }
}

void RouterManager::Simulate() {
    for (auto* router : _routers) {
        _router_threads.emplace_back(&Router::Run, router);
    }

    for (auto& thread : _router_threads) {
        thread.join();
    }
}