#include <scp.h>
#include <config.h>
#include <options.h>

#include <util/opt/options.h>

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
            auto files = ParamArr(options);
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
