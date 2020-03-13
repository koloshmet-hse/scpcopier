#include "config.h"

#include <iostream>
#include <fstream>

namespace {
    std::filesystem::path GetGlobalConfig() {
        std::filesystem::path configPath(std::getenv("HOME"));
        create_directory(configPath /= ".scpcopier");

        configPath /= "config.json";
        if (!exists(configPath) || !is_regular_file(configPath)) {
            return {};
        }

        return configPath;
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

    TTreeValue BuildEmptyConfig() {
        TTreeValue config;
        std::filesystem::path home = std::getenv("HOME");

        using namespace std::string_literals;
        config["scp_executable"] = "scp"s;
        config["login"] = home.filename();
        config["target_address"] = "127.0.0.1"s;
        config["source_root"] = home;
        config["target_root"] = home;
        config["vcs"]["path"] = "git"s;
        config["vcs"]["mode"] = "A"s;
        config["prefer_vcs"] = false;
        config["files"].Push(".scpcopier"s);

        return config;
    }
}

const std::filesystem::path& Config() try {
    static const std::filesystem::path configPath = GetConfig();
    return configPath;
} catch (const std::exception& exception) {
    std::cerr << exception.what();
    std::exit(EXIT_FAILURE);
}

const std::string& ConfigStr() try {
    static const std::string configPath = Config().string();
    return configPath;
} catch (const std::exception& exception) {
    std::cerr << exception.what();
    std::exit(EXIT_FAILURE);
}

TTreeValue LoadConfig(const std::filesystem::path& configPath) try {
    TTreeValue res;
    std::ifstream config{configPath};
    config >> TJsonIO{res};
    return res;
} catch (const std::exception& exception) {
    std::cerr << exception.what() << std::endl;
    std::exit(EXIT_FAILURE);
} catch (...) {
    std::cerr << "Unknown error" << std::endl;
    std::exit(EXIT_FAILURE);
}

TTreeValue& EmptyConfig() {
    static TTreeValue config = BuildEmptyConfig();
    return config;
}
