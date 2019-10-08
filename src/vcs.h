#pragma once

#include <util/string/utils.h>

#include <filesystem>
#include <vector>

enum class EVcs : unsigned char {
    Git
};

class TVcs {
public:
    enum class EMode : unsigned char {
        All,
        Modified,
        Added,
    };

    explicit TVcs(const std::filesystem::path& vcs, EMode mode);

    [[nodiscard]]
    std::vector<std::filesystem::path> Status() const;

private:
    std::filesystem::path VcsPath;
    EVcs Vcs;
    EMode Mode;
};
