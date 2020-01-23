#include "vcs.h"

#include <posix/subprocess/subprocess.h>

#include <util/exception/exception.h>

EVcs GetVcs(const std::filesystem::path& name) {
    if (ToLower(name.string()) == "git") {
        return EVcs::Git;
    }
    throw TException{name, " not supported"};
}

TVcs::TVcs(const std::filesystem::path& vcs, TVcs::EMode mode)
    : VcsPath{vcs}
    , Vcs{GetVcs(vcs.filename())}
    , Mode{mode}
{}

std::vector<std::filesystem::path> TVcs::Status() const {
    TSubprocess vcs = [this] {
        switch (Vcs) {
            case EVcs::Git:
                return TSubprocess{VcsPath, "status", "--porcelain"};
            default:
                throw TException{VcsPath.filename(), " status not supported"};
        }
    }();
    vcs.Execute();
    vcs.Wait();

    std::vector<std::filesystem::path> res;
    for (std::string cur; getline(vcs.Out(), cur);) {
        std::string_view view = cur;
        if (view.empty()) {
            continue;
        }

        switch (Vcs) {
            case EVcs::Git: {
                auto status = view.substr(0, 2);
                auto file = view.substr(3);

                switch (Mode) {
                    case EMode::Added:
                        if (status.front() == ' ' || status.front() == '?') {
                            break;
                        }
                    case EMode::Modified:
                        if (status.back() == '?') {
                            break;
                        }
                    case EMode::All:
                        res.emplace_back(file);
                    default:
                        break;
                }
                break;
            }
            default:
                throw TException{VcsPath.filename(), " status not supported"};
        }
    }

    return res;
}

std::filesystem::path TVcs::Root() const {
    const std::filesystem::path home = std::getenv("HOME");
    for (auto curPath = std::filesystem::current_path(); curPath != home; curPath = curPath.parent_path()) {
        auto local = curPath / ".";
        local += VcsPath.filename();
        if (exists(local) && is_directory(local)) {
            return curPath;
        }
        if (curPath == curPath.root_path()) {
            break;
        }
    }
    throw TException{"Can't find ", VcsPath.filename(), " root"};
}
