
#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

using std::vector;
using std::string;

using std::cout;
using std::cerr;
using std::endl;

using namespace std::literals::string_literals;


vector<string> SeparateLines(string const & str);
vector<string> Explode(string const & str, char delimiter);
string ReadAsString(string const & FileName);
string ReadTrimmed(string const & FileName);
vector<string> ReadAsLines(string const & FileName);
void Cat(string const & FileName, std::ostream & File);
string LeftTrimWhitespace(string s);
string RightTrimWhitespace(string s);
string TrimWhitespace(string s);
void WriteToFile(string const & FileName, string const & s, bool const append = false);
string FloatToString(float const f, int const precision = 4);
bool BeginsWith(string const & s, string const & prefix, string & remainder);
string ReplaceAll(string subject, string const & search, string const & with);
string EscapeHTML(string content);
bool FileExists(string const & FileName);
