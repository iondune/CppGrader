
#pragma once

#include "Util.hpp"
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;


bool IsHidden(fs::path const & p);
bool RemoveIfExists(fs::path const & p);
string GetExtension(string const & Path);
vector<fs::path> FindAllWithExtension(fs::path const & look_in, string const & extension);