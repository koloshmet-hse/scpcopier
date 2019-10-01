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

void PrintTabs(std::ostream& out, size_t count) {
    for (size_t i = 0; i < count * 2; ++i) {
        out << ' ';
    }
}

enum class EDirState : unsigned char {
    Processing,
    Ok,
    Error
};

std::string WrapDir(std::string_view str, EDirState state) {
    std::string res;
    switch (state) {
        case EDirState::Processing:
            res += "\033[1;33m";
            break;
        case EDirState::Ok:
            res += "\033[1;32m";
            break;
        case EDirState::Error:
            res += "\033[1;31m";
            break;
    }
    res += str;
    res += "\033[0m";
    res += '/';
    return res;
}

void ProcessResult(std::ostream& out, TSubprocess& runningScp) {
    using namespace std::literals;

    std::vector<std::string> dirStack;
    for (std::string curSent; std::getline(runningScp.Err(), curSent);) {
        std::string_view message{curSent};
        auto pattern = "Sink:"sv;
        if (message.substr(0, pattern.length()) == pattern) {
            auto mainPart = message.substr(pattern.length() + 1);
            std::vector<std::string_view> parts = Split(mainPart, " ");

            if (parts.size() == 1) {
                if (parts.front() == "E") {
                    PrintTabs(out, dirStack.size() - 1);
                    out << WrapDir(dirStack.back(), EDirState::Ok) << std::endl;
                    dirStack.pop_back();
                } else {
                    out << parts.front() << std::endl;
                }
            } else if (parts[1] == "0") {
                PrintTabs(out, dirStack.size());
                out << WrapDir(parts.back(), EDirState::Processing) << std::endl;
                dirStack.emplace_back(parts.back());
            } else {
                PrintTabs(out, dirStack.size());
                out << parts.back() << std::endl;
            }
        }
    }

    while (!dirStack.empty()) {
        PrintTabs(out, dirStack.size() - 1);
        out << WrapDir(dirStack.back(), EDirState::Error) << std::endl;
        dirStack.pop_back();
    }
}

void TScp::Upload(std::ostream& out) const {
    if (!std::filesystem::exists(SourceRoot)) {
        throw TException{"Root ", SourceRoot, " doesn't exist"};
    }
    if (!std::filesystem::is_directory(SourceRoot)) {
        throw TException{"Root ", SourceRoot, " isn't directory"};
    }

    std::vector<std::string> args{"-rqv"};
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
    ProcessResult(out, scp);
}

void TScp::Download(std::ostream& out) const {
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

    std::vector<std::string> args{"-rqv"};
    for (auto&& file : Files) {
        auto fullPath = TargetRoot;
        fullPath /= file;
        args.push_back(target + fullPath.string());
    }

    args.push_back(SourceRoot);

    TSubprocess scp{Executable, std::move_iterator{args.begin()}, std::move_iterator{args.end()}};
    scp.Execute();
    ProcessResult(out, scp);
}
