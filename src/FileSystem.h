
#pragma once

#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;


bool IsHidden(fs::path const & p)
{
	fs::path::string_type const name = p.filename();
	return (
		name != ".." &&
		name != "." &&
		name.size() > 0 &&
		name[0] == '.'
	);
}

bool RemoveIfExists(fs::path const & p)
{
	if (fs::is_regular_file(p))
	{
		fs::remove(p);
		return true;
	}

	return false;
}
