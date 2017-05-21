
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <experimental/filesystem>
#include <subprocess.hpp>

using namespace std;
namespace fs = std::experimental::filesystem;
namespace sp = subprocess;



vector<string> SeparateLines(string const & str)
{
	vector<string> Lines;
	istringstream Stream(str);
	string Line;

	while (getline(Stream, Line))
	{
		Lines.push_back(move(Line));
	}

	return Lines;
}


void required_command(std::initializer_list<const char*> const & plist)
{
	auto p = sp::Popen(plist, sp::output{sp::PIPE},  sp::error{sp::STDOUT});
	auto res = p.communicate();
	auto retcode = p.poll();
	if (retcode > 0)
	{
		throw sp::CalledProcessError("Command failed : Non zero retcode");
	}
	cout << res.first.buf.data();
}

string required_command_output(std::initializer_list<const char*> const & plist)
{
	auto p = sp::Popen(plist, sp::output{sp::PIPE},  sp::error{sp::STDOUT});
	auto res = p.communicate();
	auto retcode = p.poll();
	if (retcode > 0)
	{
		throw sp::CalledProcessError("Command failed : Non zero retcode");
	}
	return res.first.buf.data();
}


string const ExecDirectory = "/home/ian";
string const StudentsDirectory = "/home/ian/students";
string const SiteDirectory = "/var/www/html/grades";
string const TemplateDirectory = "/home/ian/csc473-gradeserver/html";


int main()
{
	string assignment = "p4";

	fs::current_path(StudentsDirectory + "/idunn01");

	cout << "Current path: " << fs::current_path() << endl;
	cout << endl;

	fs::current_path("repo");

	cout << "Running git update" << endl;
	cout << "==================" << endl;
	required_command({"git", "clean", "-d", "-x", "-f"});
	required_command({"git", "reset", "--hard",});
	required_command({"git", "fetch", "origin", "master", "--tags"});
	required_command({"git", "checkout", "master"});
	required_command({"git", "reset", "--hard", "origin/master"});
	cout << endl;

	vector<string> tags = SeparateLines(required_command_output({"git", "tag", "-l"}));

	cout << "git tags" << endl;
	cout << "========" << endl;
	if (tags.size())
	{
		for (auto tag : tags)
		{
			cout << tag << endl;
		}
	}
	else
	{
		cout << "No tags." << endl;
	}
	cout << endl;

	bool FoundTag = false;
	for (auto tag : tags)
	{
		if (tag == assignment)
		{
		cout << "Tag found, building." << endl;
			required_command({"git", "checkout", tag.c_str()});
			FoundTag = true;
		}
	}

	if (! FoundTag)
	{
		cout << "Tag not found, building master." << endl;
		required_command({"git", "checkout", "master"});
	}

	return 0;
}

