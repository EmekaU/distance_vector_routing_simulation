#include "Helper.h"
std::mutex cout_mutex;

void DEBUG_PRINT(const std::string &msg, const std::string &detail) {
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cerr << msg + ": " << detail << std::endl;
}

void DEBUG_PRINT(const std::string &msg, const int detail) {
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cerr << msg + ": " << detail << std::endl;
}

