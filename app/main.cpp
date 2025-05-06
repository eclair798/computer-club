#include <iostream>

#include "application.h"

int main(int argc, char *argv[]) {
    try {
        club::Application::Run(argc, argv);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
