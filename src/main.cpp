#include <scp.h>

#include <util/tree_value/json_io.h>

#include <util/opt/options.h>

#include <util/exception/exception.h>

#include <fstream>

std::string GetInfo(std::string_view configPath) {
    std::string res;
    res += "Version: ";
    #ifdef SCPCOPIER_VERSION
    res += SCPCOPIER_VERSION;
    #endif
    res += "\nConfig Path: ";
    res += configPath;
    return res;
}

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

using namespace std::literals;

int main(int argc, char* argv[]) {
    auto configPath = GetConfig().string();

    TOptions options{
        argc, argv,
        {
            TCommand{
                "upload", "Uploads files to server",
                TParamList<>{},
                TDefaultParam<std::string_view>{"relative_path"},
                TOpt<std::string_view>{"config", "Absolute path to config", configPath}
            },
            TCommand{
                "download", "Downloads files from server",
                TParamList<>{},
                TDefaultParam<std::string_view>{"relative_path"},
                TOpt<std::string_view>{"config", "Absolute path to config", configPath}
            },
            TCommand{
                "info", "Shows scpcopier's info",
                TParamList<>{}
            }
        }
    };

    if (options.GetCommand() == "info") {
        std::cout << GetInfo(configPath) << std::endl;
        return 0;
    }

    try {
        TScp scp{LoadConfig(options.Get<std::string_view>("config"))};

        if (options.GetCommand() == "upload" || options.GetCommand() == "download") {
            std::vector<std::string> files;
            files.reserve(options.Size());
            for (std::size_t param = 0; param < options.Size(); ++param) {
                files.emplace_back(options.Get<std::string_view>(param));
            }
            if (!files.empty()) {
                scp.SetFiles(std::move_iterator{files.begin()}, std::move_iterator{files.end()});
            }
        }

        if (options.GetCommand() == "upload") {
            scp.Upload(std::cout);
        } else if (options.GetCommand() == "download") {
            scp.Download(std::cout);
        }
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << std::endl;
    }

    return 0;
}
