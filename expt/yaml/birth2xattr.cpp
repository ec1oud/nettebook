#include <yaml-cpp/yaml.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <sys/xattr.h>

const char *cdateTag = "user.zk.creation-date";

int main(int argc, char *argv[]) {
    if (argc < 2)
        return -1;

    for (int i = 1; i < argc; ++i) {
        std::cout << argv[i] << std::endl;
        YAML::Node config = YAML::LoadFile(argv[i]);
        if (config["birth"]) {
            std::string dateStr = config["birth"].as<std::string>();
            std::cout << "birth: " << dateStr << std::endl;
            // XATTR_CREATE: don't replace it if it's already there
            int ret = setxattr(argv[i], cdateTag, dateStr.data(), dateStr.size(), XATTR_CREATE);
            if (ret < 0)
                perror(argv[i]);
        }
    }
}
