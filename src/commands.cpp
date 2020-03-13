#include "commands.h"

#include "scp.h"
#include "config.h"

#include <fstream>

namespace {
    std::string BuildInfo(std::string_view configPath) {
        std::string res;
        res += "Version: ";
        res += SCPCOPIER_VERSION;
        res += '\n';
        res += "Dated: ";
        res += SCPCOPIER_BUILD_DATE;
        res += '\n';
        res += "Config Path: ";
        res += configPath;
        return res;
    }
}

void Info(const TOptions&) {
    std::cout << BuildInfo(ConfigStr()) << std::endl;
}

void Init(const TOptions& opts) try {
    std::filesystem::path configPath = opts.Get<std::string_view>("config");

    auto path = std::filesystem::current_path();
    bool empty = opts.Get<bool>("empty");

    if (!empty) {
        try {
            create_directory(path /= ".scpcopier", configPath.parent_path());
            copy(configPath, path /= "config.json");
        } catch (...) {
            empty = true;
        }
    }
    if (empty) {
        path = std::filesystem::current_path();
        create_directory(path /= ".scpcopier");
        std::ofstream config{path /= "config.json"};
        config << TJsonIO{EmptyConfig()} << std::flush;
    }
} catch (const std::exception& exception) {
    std::cerr << exception.what() << std::endl;
}

void Download(const TOptions& opts) try {
    TScp scp{LoadConfig(opts.Get<std::string_view>("config"))};

    std::vector<std::string> files;
    files.reserve(opts.Size());
    for (std::size_t param = 0; param < opts.Size(); ++param) {
        files.emplace_back(opts.Get<std::string_view>(param));
    }
    if (!files.empty()) {
        scp.SetFiles(std::move_iterator{files.begin()}, std::move_iterator{files.end()});
    }

    scp.Download(std::cout);
} catch (const std::exception& exception) {
    std::cerr << exception.what() << std::endl;
}

void Upload(const TOptions& opts) try {
    TScp scp{LoadConfig(opts.Get<std::string_view>("config"))};

    std::vector<std::string> files;
    files.reserve(opts.Size());
    for (std::size_t param = 0; param < opts.Size(); ++param) {
        files.emplace_back(opts.Get<std::string_view>(param));
    }
    if (!files.empty()) {
        scp.SetFiles(std::move_iterator{files.begin()}, std::move_iterator{files.end()});
    }

    scp.Upload(std::cout);
} catch (const std::exception& exception) {
    std::cerr << exception.what() << std::endl;
}
