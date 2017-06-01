
#define SUBPROCESS_HPP_IMPLEMENTATION

#include "Util.hpp"
#include "Process.hpp"
#include "FileSystem.hpp"
#include "Directories.hpp"
#include "Test.hpp"
#include "Grader.hpp"

#include <deque>

#include <json.hpp>


class usage_exception : public std::runtime_error
{

public:

	usage_exception(string const & what)
		: std::runtime_error(what)
	{}

};

class parse_exception : public std::runtime_error
{

public:

	parse_exception(string const & assignment, string const & what)
		: Assignment(assignment), std::runtime_error(what)
	{}

	string Assignment;

};

void PrintUsage(string const & exec_name)
{
	cerr << "usage: " << exec_name << " all" << endl;
	cerr << "   or: " << exec_name << " [student] [assignment]" << endl;
}

vector<Test> ParseSuite(string const & assignment)
{
	vector<Test> TestSuite;

	try
	{
		std::ifstream i(AllTestsDirectory + assignment + "/" + "tests.json");
		nlohmann::json j;
		i >> j;
		TestSuite = j.get<vector<Test>>();
	}
	catch (std::invalid_argument const & e)
	{
		throw parse_exception(assignment, e.what());
	}

	return TestSuite;
}

void GradeAll()
{
	cout << "################################################################################" << endl;
	cout << "Performing bulk grade of all students/assignments." << endl;
	cout << endl << endl;

	vector<string> assignments = ReadAsLines(ExecDirectory + "assignments");
	vector<string> students = ReadAsLines(AllStudentsDirectory + "list");

	cout << "Assignments" << endl;
	cout << "===========" << endl;
	for (auto assignment : assignments)
	{
		cout << assignment << endl;
	}
	cout << endl;

	cout << "Students" << endl;
	cout << "========" << endl;
	for (auto student : students)
	{
		cout << student << endl;
	}
	cout << endl;

	for (string const & assignment : assignments)
	{
		vector<Test> const TestSuite = ParseSuite(assignment);

		for (string const & student : students)
		{
			Grader g(student, assignment, TestSuite);
			g.Run();
		}

		cout << endl << endl;
	}
}

void Run(std::deque<string> Arguments)
{
	if (Arguments.size() == 0)
	{
		throw usage_exception("not enough arguments.");
	}

	if (Arguments.front() == "--all")
	{
		GradeAll();
		return;
	}

	string student;
	string assignment;
	bool Regrade = false;

	while (Arguments.size())
	{
		string const Argument = Arguments.front();
		Arguments.pop_front();

		string Remainder;
		if (BeginsWith(Argument, "--student=", Remainder))
		{
			student = Remainder;
		}
		else if (BeginsWith(Argument, "--assignment=", Remainder))
		{
			assignment = Remainder;
		}
		else if (Argument == "--regrade")
		{
			Regrade = true;
		}
	}

	if (student == "")
	{
		throw usage_exception("no student specified.");
	}

	if (assignment == "")
	{
		throw usage_exception("no assignment specified.");
	}

	vector<Test> const TestSuite = ParseSuite(assignment);

	Grader g(student, assignment, TestSuite);
	g.Regrade = Regrade;
	g.Run();
}

int main(int argc, char const ** argv)
{
	std::deque<string> Arguments;
	for (int i = 0; i < argc; ++ i)
	{
		Arguments.push_back(argv[i]);
	}

	string ExecName = "CppGrader";
	if (Arguments.size() > 0)
	{
		ExecName = Arguments.front();
		Arguments.pop_front();
	}

	try
	{
		Run(Arguments);
	}
	catch (usage_exception const & e)
	{
		cout << "Incorrect arguments: " << e.what() << endl;
		PrintUsage(ExecName);
		return 1;
	}
	catch (parse_exception const & e)
	{
		cout << "Failed to parse test suite for assignment '" << e.Assignment << "'" << endl;
		cout << e.what() << endl;
		return 1;
	}

	return 0;
}
