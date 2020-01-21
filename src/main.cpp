#include <scp.h>

#include <util/tree_value/json_io.h>

#include <util/opt/options.h>

#include <util/exception/exception.h>

#include <fstream>

std::ifstream GetGlobalConfig() {
    std::filesystem::path configPath(std::getenv("HOME"));
    configPath /= ".scpcopier";
    if (!exists(configPath)) {
        create_directory(configPath);
    }
    if (!is_directory(configPath)) {
        throw TException{
            "You already have ~/.scpcopier and it isn't directory, please move it somewhere else to continue"
        };
    }
    configPath /= "config.json";
    if (!exists(configPath) || !is_regular_file(configPath)) {
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
        if (exists(local) && is_directory(local)) {
            local /= "config.json";
            if (exists(local) && is_regular_file(local)) {
                return std::ifstream{local};
            }
        }
    }
    return GetGlobalConfig();
}

TTreeValue LoadConfig() {
    TTreeValue res;
    try {
        auto config = GetLocalConfig();
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
    TOptions options{
        argc, argv,
        {
            TCommand{
                "upload", "Uploads files to server",
                TParamList<>{},
                TDefaultParam<std::string_view>{"relative_path"},
            },
            TCommand{
                "download", "Downloads files from server",
                TParamList<>{},
                TDefaultParam<std::string_view>{"relative_path"},
            }
        }
    };

    try {
        TScp scp{LoadConfig()};
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
