//
// Created by cleon on 22-9-10.
//

#include <iostream>

#include "InetAddress.h"


int main() {
    InetAddress addr(8090);
    std::cout << addr.toIpPort() << std::endl;

    return 0;
}