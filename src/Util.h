
#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>

using std::vector;
using std::string;


vector<string> SeparateLines(string const & str)
{
	vector<string> Lines;
	std::istringstream Stream(str);
	string Line;

	while (getline(Stream, Line))
	{
		Lines.push_back(move(Line));
	}

	return Lines;
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