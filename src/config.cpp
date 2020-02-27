#include "config.h"

namespace {
    std::filesystem::path GetGlobalConfig() {
        std::filesystem::path configPath(std::getenv("HOME"));
        configPath /= ".scpcopier";
        if (!exists(configPath)) {
            create_directory(configPath);
        }
        if (!is_directory(configPath)) {
            return {};
        }
        configPath /= "config.json";
        if (!exists(configPath) || !is_regular_file(configPath)) {
            return {};
        }

        return configPath;
    }
}

std::filesystem::path GetConfig() {
    const std::filesystem::path home = std::getenv("HOME");
    for (auto curPath = std::filesystem::current_path(); curPath != home; curPath = curPath.parent_path()) {
        auto local = curPath / ".scpcopier";
        if (exists(local) && is_directory(local)) {
            local /= "config.json";
            if (exists(local) && is_regular_file(local)) {
                return local;
            }
        }
        if (curPath == curPath.root_path()) {
            break;
        }
    }
    return GetGlobalConfig();
}

TTreeValue LoadConfig(const std::filesystem::path& configPath) {
    TTreeValue res;
    try {
        std::ifstream config{configPath};
        config >> TJsonIO{res};
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << std::endl;
        std::exit(1);
    } catch (...) {
        std::cerr << "Unknown error" << std::endl;
        std::exit(1);
    }
    return res;
}
