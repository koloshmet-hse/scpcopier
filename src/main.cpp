#include <scp.h>

#include <util/tree_value/json_io.h>

#include <util/opt/options.h>

#include <util/string/utils.h>
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
        TParamList<>{},
        TDefaultParam<std::string_view>{"relative_path"},
        TOpt<bool>{"u", "Uploads files to server. Never use with -d"},
        TOpt<bool>{"d", "Downloads files from server. Never use with -u"}
    };

    try {
        TScp scp{LoadConfig()};
        if (options.Get<bool>("u") ^ options.Get<bool>("d")) {
            std::vector<std::string> files;
            files.reserve(options.Size() - 1);
            for (std::size_t param = 1; param < options.Size(); ++param) {
                files.emplace_back(options.Get<std::string_view>(param));
            }
            if (!files.empty()) {
                scp.SetFiles(std::move_iterator{files.begin()}, std::move_iterator{files.end()});
            }

            if (options.Get<bool>("u")) {
                scp.Upload(std::cout);
            } else {
                scp.Download(std::cout);
            }
        } else {
            throw TException{"Put one argument '-u' or '-d', for more information use --help"};
        }
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << std::endl;
    }

    return 0;
}
