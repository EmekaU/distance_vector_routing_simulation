#include "helper.h"

void DEBUG_PRINT(const std::string &msg, const std::string &detail) {
    std::cerr << msg + ": " << detail << std::endl;
}

void DEBUG_PRINT(const std::string &msg, const int detail) {
    std::cerr << msg + ": " << detail << std::endl;
}

