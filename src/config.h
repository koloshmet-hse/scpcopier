#pragma once

#include <util/tree_value/json_io.h>

#include <filesystem>
#include <iostream>
#include <fstream>

std::filesystem::path GetConfig();

TTreeValue LoadConfig(const std::filesystem::path& configPath);
