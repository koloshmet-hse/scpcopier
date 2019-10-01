#include <scp.h>

#include <util/tree_value/json_io.h>

#include <util/string/utils.h>
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

void PrintTabs(size_t count) {
    for (size_t i = 0; i < count * 2; ++i) {
        std::cout << ' ';
    }
}

std::string WrapDir(std::string_view str, bool finished = false) {
    std::string res;
    if (!finished) {
        res += "\033[1;33m";
    } else {
        res += "\033[1;32m";
    }
    res += str;
    res += "\033[0m";
    return res;
}

void ProcessResult(TSubprocess& runningScp) {
    std::vector<std::string> dirStack;
    for (std::string curSent; std::getline(runningScp.Err(), curSent);) {
        std::string_view message{curSent};
        auto pattern = "Sink:"sv;
        if (message.substr(0, pattern.length()) == pattern) {
            auto mainPart = message.substr(pattern.length() + 1);
            std::vector<std::string_view> parts = Split(mainPart, " ");

            if (parts.size() == 1) {
                if (parts.front() == "E") {
                    PrintTabs(dirStack.size() - 1);
                    std::cout << WrapDir(dirStack.back(), true) << std::endl;
                    dirStack.pop_back();
                } else {
                    std::cout << parts.front() << std::endl;
                }
            } else if (parts[1] == "0") {
                PrintTabs(dirStack.size());
                std::cout << WrapDir(parts.back()) << std::endl;
                dirStack.emplace_back(parts.back());
            } else {
                PrintTabs(dirStack.size());
                std::cout << parts.back() << std::endl;
            }
        }
    }
}

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

        ProcessResult(runningScp);
        runningScp.Wait();
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << std::endl;
    }

    return 0;
}
