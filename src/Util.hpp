
#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>

using std::vector;
using std::string;
using namespace std::literals::string_literals;


vector<string> SeparateLines(string const & str);
vector<string> Explode(string const & str, char delimiter);
string ReadAsString(string const & FileName);
vector<string> ReadAsLines(string const & FileName);
string Cat(string const & FileName, std::ostream & File);
string LeftTrimWhitespace(string s);
string RightTrimWhitespace(string s);
string TrimWhitespace(string s);
void WriteToFile(string const & FileName, string const & s);
