//
// Created by cleon on 22-9-10.
//

#include <iostream>

#include "Timestamp.h"

int main() {
    std::cout << Timestamp::now().toString() << std::endl;
    return 0;
}