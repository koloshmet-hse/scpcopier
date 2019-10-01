#include <scp.h>

#include <util/tree_value/json_io.h>

#include <util/exception/exception.h>

#include <fstream>
#include <iostream>

std::ifstream GetConfig() {
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

using namespace std::literals;

int main(int argc, char* argv[]) {
    TTreeValue configJson;
    try {
        auto config = GetConfig();
        config >> TJsonIO{configJson};
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << std::endl;
    }

    try {
        TScp scp{configJson};
        if (argc < 2) {
            throw TException{"Put one argument 'u' - Upload or 'd' - Download"};
        }
        auto runningScp = [&scp, argv] {
            std::string arg{argv[1]};
            if (arg.empty() || (std::tolower(arg.front()) != 'u' && std::tolower(arg.front()) != 'd')) {
                throw TException{"Put one argument 'u' - Upload or 'd' - Download"};
            }
            if (std::tolower(arg.front()) == 'u') {
                return scp.Upload();
            } else {
                return scp.Download();
            }
        }();

        std::string curSent;
        while (std::getline(runningScp.Err(), curSent)) {
            std::string_view message{curSent};
            auto pattern = "Sink:"sv;
            if (message.substr(0, pattern.length()) == pattern) {
                std::cout << message.substr(pattern.length()) << std::endl;
            }
        }
        runningScp.Wait();
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << std::endl;
    }

    return 0;
}
