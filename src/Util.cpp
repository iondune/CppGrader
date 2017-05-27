
#include "Util.hpp"


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
	std::string String;

	FileHandle.seekg(0, std::ios::end);
	String.reserve((uint) FileHandle.tellg());
	FileHandle.seekg(0, std::ios::beg);

	String.assign((std::istreambuf_iterator<char>(FileHandle)), std::istreambuf_iterator<char>());

	return String;
}

vector<string> ReadAsLines(string const & FileName)
{
	return SeparateLines(ReadAsString(FileName));
}

string Cat(string const & FileName, std::ostream & File)
{
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

void WriteToFile(string const & FileName, string const & s)
{
	std::ofstream file;
	file.open(FileName);

	file << s;
	file.close();
}
