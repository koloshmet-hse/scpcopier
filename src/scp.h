#pragma once

#include <posix/subprocess/subprocess.h>

#include <util/tree_value/tree_value.h>

class TScp {
public:
    explicit TScp(const TTreeValue& config);

    void Upload(std::ostream& out) const;

    void Download(std::ostream& out) const;

private:
    const std::filesystem::path Executable;
    std::vector<std::string> Files;
    std::string Target;
    std::string Login;
    std::filesystem::path SourceRoot;
    std::filesystem::path TargetRoot;
};
