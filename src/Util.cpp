
#include "Util.hpp"
#include <iomanip>


vector<string> SeparateLines(string const & str)
{
	vector<string> Lines;
	std::istringstream Stream(str);
	string Line;

	while (getline(Stream, Line))
	{
		Lines.push_back(std::move(Line));
	}

	return Lines;
}

vector<string> Explode(string const & str, char delimiter)
{
	vector<string> Words;
	std::istringstream Stream(str);
	string Word;

	while (getline(Stream, Word, delimiter))
	{
		Words.push_back(std::move(Word));
	}

	return Words;
}

string ReadAsString(string const & FileName)
{
	std::ifstream FileHandle(FileName);
	if (! FileHandle)
	{
		cout << "Could not open file: '" << FileName << "'" << endl;
		return "";
	}

	std::string String;

	FileHandle.seekg(0, std::ios::end);
	String.reserve((uint) FileHandle.tellg());
	FileHandle.seekg(0, std::ios::beg);

	String.assign((std::istreambuf_iterator<char>(FileHandle)), std::istreambuf_iterator<char>());

	return String;
}

string ReadTrimmed(string const & FileName)
{
	return TrimWhitespace(ReadAsString(FileName));
}

vector<string> ReadAsLines(string const & FileName)
{
	return SeparateLines(ReadAsString(FileName));
}

void Cat(string const & FileName, std::ostream & File)
{
	if (! File)
	{
		cout << "Could not cat '" << FileName << "' - output file is not open" << endl;
		return;
	}
	File << ReadAsString(FileName);
}

string LeftTrimWhitespace(string s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

string RightTrimWhitespace(string s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}

string TrimWhitespace(string s)
{
	return RightTrimWhitespace(LeftTrimWhitespace(s));
}

void WriteToFile(string const & FileName, string const & s, bool const append)
{
	std::ofstream file;
	if (append)
	{
		file.open(FileName, std::ios_base::app);
	}
	else
	{
		file.open(FileName);
	}

	file << s;
	file.close();
}

string FloatToString(float const f, int const precision)
{
	std::ostringstream s;
	// s << std::setiosflags(std::ios::fixed);
	s << std::setprecision(precision);
	s << f;
	return s.str();
}

bool BeginsWith(string const & s, string const & prefix, string & remainder)
{
	if (s.size() < prefix.size())
	{
		return false;
	}

	if (s.substr(0, prefix.size()) == prefix)
	{
		remainder = s.substr(prefix.size());
		return true;
	}

	return false;
}

string ReplaceAll(string subject, string const & search, string const & with)
{
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != string::npos)
	{
		subject.replace(pos, search.length(), with);
		pos += with.length();
	}
	return subject;
}

string EscapeHTML(string content)
{
	content = ReplaceAll(content, "&", "&amp;");
	content = ReplaceAll(content, "<", "&lt;");
	content = ReplaceAll(content, ">", "&gt;");
	return content;
}

bool FileExists(string const & FileName)
{
	std::ifstream ifile(FileName);
	return ifile.good();
}
