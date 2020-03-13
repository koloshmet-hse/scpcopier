#include "commands.h"
#include "config.h"

#include <util/opt/options.h>

using namespace std::literals;

int main(int argc, char* argv[]) {
    TOptions options{
        argc, argv,
        {
            TCommand{
                "upload", "Uploads files to server",
                TParamList<>{},
                TDefaultParam<std::string_view>{"relative_path"},
                TOpt<std::string_view>{"config", "Absolute path to config", ConfigStr()}
            },
            TCommand{
                "download", "Downloads files from server",
                TParamList<>{},
                TDefaultParam<std::string_view>{"relative_path"},
                TOpt<std::string_view>{"config", "Absolute path to config", ConfigStr()}
            },
            TCommand{
                "info", "Shows scpcopier info",
                TParamList<>{}
            },
            TCommand{
                "init", "Initializes scpcopier config",
                TParamList<>{},
                TOpt<bool>{"empty", "Generate empty config"},
                TOpt<std::string_view>{"config", "Absolute path to config", ConfigStr()}
            }
        }
    };

    if (options.GetCommand() == "info") {
        Info(options);
    } else if (options.GetCommand() == "init") {
        Init(options);
    } else if (options.GetCommand() == "download") {
        Download(options);
    } else if (options.GetCommand() == "upload") {
        Upload(options);
    }

    return 0;
}
