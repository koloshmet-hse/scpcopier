#include "scp.h"

#include <util/string/utils.h>
#include <util/exception/exception.h>

TScp::TScp(const TTreeValue& config)
    : Executable(ToString(config["scp_executable"]))
    , Files{config["files"].AsArray().begin(), config["files"].AsArray().end()}
    , Target{ToString(config["target_address"])}
    , Login{ToString(config["login"])}
    , SourceRoot{ToString(config["source_root"])}
    , TargetRoot{ToString(config["target_root"])}
{}

TSubprocess TScp::Upload() const {
    if (!std::filesystem::exists(SourceRoot)) {
        throw TException{"Root ", SourceRoot, " doesn't exist"};
    }
    if (!std::filesystem::is_directory(SourceRoot)) {
        throw TException{"Root ", SourceRoot, " isn't directory"};
    }

    std::vector<std::string> args{"-r"};
    for (auto&& file : Files) {
        auto fullPath = SourceRoot;
        fullPath /= file;
        if (!std::filesystem::exists(fullPath)) {
            throw TException{"File ", fullPath, " doesn't exist"};
        }
        args.push_back(fullPath);
    }

    std::string target;
    target += Login;
    target += '@';
    target += Target;
    target += ':';
    target += TargetRoot;

    args.push_back(std::move(target));

    TSubprocess scp{Executable, std::move_iterator{args.begin()}, std::move_iterator{args.end()}};
    scp.Execute();
    return scp;
}

TSubprocess TScp::Download() const {
    if (!std::filesystem::exists(SourceRoot)) {
        throw TException{"Root ", SourceRoot, " doesn't exist"};
    }
    if (!std::filesystem::is_directory(SourceRoot)) {
        throw TException{"Root ", SourceRoot, " isn't directory"};
    }

    std::string target;
    target += Login;
    target += '@';
    target += Target;
    target += ':';

    std::vector<std::string> args{"-r"};
    for (auto&& file : Files) {
        auto fullPath = TargetRoot;
        fullPath /= file;
        args.push_back(target + fullPath.string());
    }

    args.push_back(SourceRoot);

    TSubprocess scp{Executable, std::move_iterator{args.begin()}, std::move_iterator{args.end()}};
    scp.Execute();
    return scp;
}
