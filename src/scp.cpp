#include "scp.h"

#include <vcs.h>

#include <util/string/utils.h>
#include <util/exception/exception.h>

TVcs::EMode GetMode(std::string_view mode) {
    switch (std::tolower(mode.front())) {
        case 'a':
            return TVcs::EMode::Added;
        case 'm':
            return TVcs::EMode::Modified;
        default:
            return TVcs::EMode::All;
    }
}

TScp::TScp(const TTreeValue& config)
    : Executable(ToString(config["scp_executable"]))
    , Files{}
    , Target{ToString(config["target_address"])}
    , Login{ToString(config["login"])}
    , Root{}
    , SourceRoot{ToString(config["source_root"])}
    , TargetRoot{ToString(config["target_root"])}
{
    const std::filesystem::path home = std::getenv("HOME");
    for (Root = std::filesystem::current_path(); Root != home; Root = Root.parent_path()) {
        auto local = Root / ".scpcopier";
        if (exists(local) && is_directory(local)) {
            break;
        }
    }

    bool preferVcs = config.Contains("vcs") && !config.Contains("files");
    if (config.Contains("vcs") && config.Contains("files")) {
        if (config.Contains("prefer_vcs")) {
            preferVcs = static_cast<bool>(config["prefer_vcs"]);
        }
    }

    if (preferVcs) {
        TVcs vcs{ToString(config["vcs"]["path"]), GetMode(ToString(config["vcs"]["mode"]))};
        
        current_path(SourceRoot);
        SourceRoot = vcs.Root();

        auto files = vcs.Status();

        for (auto&& cur : files) {
            Files.push_back(cur.string());
        }
    } else {
        for (auto&& cur : config["files"].AsArray()) {
            Files.push_back(ToString(cur));
        }
    }
}

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
    for (std::string curSent; getline(runningScp.Err(), curSent);) {
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

constexpr const char SSH_AUTH[] = "SSH_AUTH_SOCK";

void TScp::Upload(std::ostream& out) const {
    if (!exists(SourceRoot)) {
        throw TException{"Root ", SourceRoot, " doesn't exist"};
    }
    if (!is_directory(SourceRoot)) {
        throw TException{"Root ", SourceRoot, " isn't directory"};
    }

    auto args = UploadParams(Target, SourceRoot, TargetRoot);
    TSubprocess scp{Executable, std::move_iterator{args.begin()}, std::move_iterator{args.end()}};
    scp.AddEnv(SSH_AUTH, std::getenv(SSH_AUTH));
    scp.Execute();
    ProcessResult(out, scp);
    scp.Wait();
}

void TScp::Download(std::ostream& out) const {
    if (!exists(SourceRoot)) {
        throw TException{"Root ", SourceRoot, " doesn't exist"};
    }
    if (!is_directory(SourceRoot)) {
        throw TException{"Root ", SourceRoot, " isn't directory"};
    }

    auto args = DownloadParams(Target, TargetRoot, SourceRoot);
    TSubprocess scp{Executable, std::move_iterator{args.begin()}, std::move_iterator{args.end()}};
    scp.AddEnv(SSH_AUTH, std::getenv(SSH_AUTH));
    scp.Execute();
    ProcessResult(out, scp);
    scp.Wait();
}

std::vector<std::string> TScp::UploadParams(
    std::string_view to,
    const std::filesystem::path& fromRoot,
    const std::filesystem::path& toRoot) const
{
    std::vector<std::string> args{"-rqv"};
    for (auto&& file : Files) {
        auto fullPath = fromRoot;
        fullPath /= file;
        if (!exists(fullPath)) {
            throw TException{"File ", fullPath, " doesn't exist"};
        }
        args.push_back(std::move(fullPath));
    }

    std::string target;
    target += Login;
    target += '@';
    target += to;
    target += ':';
    target += toRoot;

    args.push_back(std::move(target));
    return args;
}

std::vector<std::string> TScp::DownloadParams(
    std::string_view from,
    const std::filesystem::path& fromRoot,
    const std::filesystem::path& toRoot) const
{
    std::string target;
    target += Login;
    target += '@';
    target += from;
    target += ':';

    std::vector<std::string> args{"-rqv"};
    for (auto&& file : Files) {
        auto fullPath = fromRoot;
        fullPath /= file;
        args.push_back(target + fullPath.string());
    }

    args.push_back(toRoot);
    return args;
}
