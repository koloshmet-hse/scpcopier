#include <scp.h>

#include <util/tree_value/json_io.h>

#include <util/string/utils.h>
#include <util/exception/exception.h>

#include <fstream>
#include <iostream>

std::ifstream GetGlobalConfig() {
    std::filesystem::path configPath(std::getenv("HOME"));
    configPath /= ".scpcopier";
    if (!std::filesystem::exists(configPath)) {
        std::filesystem::create_directory(configPath);
    }
    if (!std::filesystem::is_directory(configPath)) {
        throw TException{
            "You already have ~/.scpcopier and it isn't directory, please move it somewhere else to continue"
        };
    }
    configPath /= "config.json";
    if (!std::filesystem::exists(configPath) || !std::filesystem::is_regular_file(configPath)) {
        throw TException{
            "You don't have ~/.scpcopier/config.json or it isn't regular file, please create it to continue"
        };
    }

    return std::ifstream{configPath};
}

std::ifstream GetLocalConfig() {
    const std::filesystem::path home = std::getenv("HOME");
    for (auto curPath = std::filesystem::current_path(); curPath != home; curPath = curPath.parent_path()) {
        auto local = curPath / ".scpcopier";
        if (std::filesystem::exists(local) && std::filesystem::is_directory(local)) {
            local /= "config.json";
            if (std::filesystem::exists(local) && std::filesystem::is_regular_file(local)) {
                return std::ifstream{local};
            }
        }
    }
    return GetGlobalConfig();
}

using namespace std::literals;

int main(int argc, char* argv[]) {
    TTreeValue configJson;
    try {
        auto config = GetLocalConfig();
        config >> TJsonIO{configJson};
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << std::endl;
    }

    try {
        TScp scp{configJson};
        if (argc < 2) {
            throw TException{"Put one argument 'u' - Upload or 'd' - Download"};
        }
        std::string_view arg{argv[1]};
        if (arg.empty() || (std::tolower(arg.front()) != 'u' && std::tolower(arg.front()) != 'd')) {
            throw TException{"Put one argument 'u' - Upload or 'd' - Download"};
        }

        if (std::tolower(arg.front()) == 'u') {
            scp.Upload(std::cout);
        } else {
            scp.Download(std::cout);
        }
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << std::endl;
    }

    return 0;
}
