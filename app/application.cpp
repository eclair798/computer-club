#include "application.h"

namespace club {

void Application::Run(int argc, char* argv[]) {
    if (argc < 2) {
        throw std::invalid_argument(
            "Error. Few arguments. Only path to the input file is required");
    }
    if (argc > 2) {
        throw std::invalid_argument(
            "Error. Too many arguments. Only path to the input file is required");
    }
    Path inputPath = argv[1];
    Club club(inputPath);
}

}  // namespace club
