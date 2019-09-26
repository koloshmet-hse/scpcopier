#pragma once

#include <posix/subprocess/subprocess.h>

#include <util/tree_value/tree_value.h>

class TScp {
public:
    explicit TScp(const TTreeValue& config);

    [[nodiscard]]
    TSubprocess Upload() const;

    [[nodiscard]]
    TSubprocess Download() const;

private:
    const std::filesystem::path Executable;
    std::vector<std::string> Files;
    std::string Target;
    std::string Login;
    std::filesystem::path SourceRoot;
    std::filesystem::path TargetRoot;
};
