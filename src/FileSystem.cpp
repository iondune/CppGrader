
#include "FileSystem.hpp"


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

string GetExtension(string const & Path)
{
	size_t const Index = Path.find_last_of(".");
	if (Index == string::npos)
	{
		return "";
	}
	else
	{
		return Path.substr(Index + 1);
	}
}

vector<fs::path> FindAllWithExtension(fs::path const & look_in, string const & extension)
{
	vector<fs::path> files;

	for (auto entry : fs::recursive_directory_iterator(look_in))
	{
		auto p = entry.path();

		if (! IsHidden(p))
		{
			if (fs::is_regular_file(p) && GetExtension(p.string()) == extension)
			{
				files.push_back(p);
			}
		}
	}

	return files;
}
