#include <yaml-cpp/yaml.h>

#include <chrono>
#include <fstream>
#include <iostream>

int main(int argc, char *argv[]) {
    if (argc < 2)
        return -1;

    YAML::Node config = YAML::LoadFile(argv[1]);

    if (config["birth"]) {
        std::cout << "birth: " << config["birth"].as<std::string>() << "\n";
    }
}
