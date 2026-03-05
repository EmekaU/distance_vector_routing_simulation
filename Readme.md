# Distance Vector Routing Simulation

A multi-threaded TCP-based simulation of the distance vector routing protocol using the Bellman-Ford algorithm, written in C++20.

## Overview

This program simulates a network of routers that discover optimal paths to all destinations through periodic exchange of routing tables. Each router runs in its own thread, communicates with neighbors over TCP sockets, and uses a `select`-based event loop to handle concurrent I/O.

## Architecture

**Router** — each router is an independent entity running in its own thread. It maintains a routing table, a set of TCP connections to its direct neighbors, and a `select`-based event loop that handles accepting connections, receiving route advertisements, and periodically broadcasting its own routing table.

**RouterManager** — the orchestrator that reads the network topology from a configuration file, constructs router instances, initializes their listening sockets, and spawns their threads.

**Bellman-Ford convergence** — routers periodically broadcast their full routing table to all neighbors. When a router receives a neighbor's table, it checks whether any destination is reachable at lower cost through that neighbor and updates its own table accordingly. Over several broadcast cycles, all routers converge to the global shortest paths.

## Connection Establishment

To avoid duplicate TCP connections between neighbors, routers use a lexicographic rule: only the router with the alphabetically smaller name initiates the connection. The other side accepts and registers the connection when it receives the introduction message.

## Configuration

Network topology is defined in `config/initial_topology.txt`. Each line specifies a bidirectional link between two routers and its cost:

```
A B 1
A C 5
B C 2
B D 4
C D 1
C E 6
D E 3
```

This produces the following network:

```
A --- 1 --- B
|           |
5           4
|           |
C --- 1 --- D
|           |
6           3
|           |
+--- E -----+
```

## Building

Requires CMake 4.1+ and a C++20 compatible compiler.

```bash
mkdir build && cd build
cmake ..
make
```

## Running

```bash
./distance_vector_routing_simul
```

Routers will initialize, establish connections, and begin exchanging routing tables. Each router periodically prints its routing table to the terminal, showing convergence progress.

## Project Structure

```
├── main.cpp                      # Entry point
├── config/
│   └── initial_topology.txt      # Network topology definition
├── router/
│   ├── Router.h                  # Router class declaration
│   ├── Router.cpp                # Router implementation
│   ├── helper.h                  # Shared types and utilities
│   └── helper.cpp                # Utility implementations
├── ControlPane/
│   ├── RouterManager.h           # Manager class declaration
│   └── RouterManager.cpp         # Manager implementation
└── CMakeLists.txt
```

## Wire Protocol

Routers communicate using simple text-based serialization over TCP.

**Introduction message** (sent on new connection): `name:cost|port`

**Route advertisement** (sent periodically): `name|dest1:cost1,dest2:cost2,...`

## Possible Extensions

- **Poison reverse** — prevent count-to-infinity by advertising infinite cost for routes learned through the receiving neighbor
- **Dynamic topology changes** — add or remove links at runtime via a control plane
- **GUI visualization** — real-time network topology display using Dear ImGui
- **Next-hop tracking** — track which neighbor a route goes through, not just the cost