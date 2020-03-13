#pragma once

#include <util/tree_value/json_io.h>

#include <filesystem>

const std::filesystem::path& Config();

const std::string& ConfigStr();

TTreeValue LoadConfig(const std::filesystem::path& configPath);

TTreeValue& EmptyConfig();
