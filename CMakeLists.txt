cmake_minimum_required(VERSION 4.1)
project(distance_vector_routing_simul)

set(CMAKE_CXX_STANDARD 20)

add_executable(distance_vector_routing_simul main.cpp
        router/Router.cpp
        router/Router.h
        router/helper.cpp
        router/helper.h
)
