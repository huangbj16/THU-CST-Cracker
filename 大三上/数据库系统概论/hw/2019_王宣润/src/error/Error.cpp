#include "Error.h"

#include <iostream>
#include <cstdlib>

Error::Error(const std::string &msg) {
    std::cout << msg << std::endl;
    exit(0);
}
