#pragma once

#include <posix/subprocess/subprocess.h>

#include <util/tree_value/tree_value.h>

class TScp {
public:
    explicit TScp(const TTreeValue& config);

    void Upload(std::ostream& out) const;

    void Download(std::ostream& out) const;

    template <typename TIter>
    void SetFiles(TIter beg, TIter end) {
        Files.resize(std::distance(beg, end));
        copy(beg, end, Files.begin());
    }

private:
    [[nodiscard]]
    std::vector<std::string> UploadParams(
        std::string_view to,
        const std::filesystem::path& fromRoot,
        const std::filesystem::path& toRoot) const;

    [[nodiscard]]
    std::vector<std::string> DownloadParams(
        std::string_view from,
        const std::filesystem::path& fromRoot,
        const std::filesystem::path& toRoot) const;

private:
    std::filesystem::path Executable;
    std::vector<std::string> Files;
    std::string Target;
    std::string Login;
    std::filesystem::path SourceRoot;
    std::filesystem::path TargetRoot;
};
