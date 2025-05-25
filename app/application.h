#pragma once

#include <stdexcept>

#include "club.h"

namespace club {

class Application {
    using Path = std::filesystem::path;
    using File = std::ifstream;

public:
    static void Run(int argc, char *argv[]);

private:
    Club club_;
    Path inputPath_;
};

}  // namespace club
