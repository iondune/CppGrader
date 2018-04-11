
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
	cerr << endl;
	cerr << "usage: " << exec_name << " [options]" << endl;
	cerr << endl;
	cerr << "   options:" << endl;
	cerr << "       --all                      grade all students (otherwise, --student required)" << endl;
	cerr << "       --regrade                  force a grade of the latest commit" << endl;
	cerr << "       --dry                      dry run, merely print what grading tasks would be peformed" << endl;
	cerr << "       --student=<username>       grade only a particular student" << endl;
	cerr << "       --assignment=<assignment>  grade only a particular assignment" << endl;
	cerr << "       --commit=<commit>          grade only a particular commit (--student also required)" << endl;
	cerr << endl;
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

void Run(std::deque<string> Arguments)
{
	if (Arguments.size() == 0)
	{
		throw usage_exception("not enough arguments.");
	}

	string student;
	string assignment;
	string commit;

	bool All = false;
	bool Regrade = false;
	bool Dry = false;

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
		else if (BeginsWith(Argument, "--commit=", Remainder))
		{
			commit = Remainder;
		}
		else if (Argument == "--all")
		{
			All = true;
		}
		else if (Argument == "--regrade")
		{
			Regrade = true;
		}
		else if (Argument == "--dry")
		{
			Dry = true;
		}
		else
		{
			throw usage_exception("unknown option '" + Argument + "'");
		}
	}

	cout << "################################################################################" << endl;
	cout << endl;
	cout << "Starting grade run ..." << endl;
	auto current_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	cout << "Current time: " << std::ctime(&current_time);
	cout << endl;

	vector<string> students;
	vector<string> assignments;

	if (student == "")
	{
		if (All)
		{
			students = ReadAsLines(AllStudentsDirectory + "list");

			cout << "Students" << endl;
			cout << "========" << endl;
			for (auto student : students)
			{
				cout << student << endl;
			}
			cout << endl;
		}
		else
		{
			throw usage_exception("no student specified.");
		}
	}
	else
	{
		students.push_back(student);
		cout << "Student: " << student << endl;
	}

	if (students.size() > 1 && commit != "")
	{
		throw usage_exception("can't grade specific hash for multiple students.");
	}

	if (assignment == "")
	{
		if (All)
		{
			assignments = ReadAsLines(ExecDirectory + "assignments");

			cout << "Assignments" << endl;
			cout << "===========" << endl;
			for (auto assignment : assignments)
			{
				cout << assignment << endl;
			}
			cout << endl;
		}
		else
		{
			throw usage_exception("no assignment specified.");
		}
	}
	else
	{
		assignments.push_back(assignment);
		cout << "Assignment: " << assignment << endl;
	}

	cout << endl;

	for (string const & assignment : assignments)
	{
		vector<Test> const TestSuite = ParseSuite(assignment);

		for (string const & student : students)
		{
			Grader g(student, assignment, TestSuite);

			string HashToGrade = g.GetLatestHash();
			if (commit != "")
			{
				HashToGrade = commit;
			}

			bool const WorkToDo = g.CheckWorkToDo(HashToGrade);

			string Status = "grade";
			if (! WorkToDo)
			{
				Status = (Regrade ? "force-run" : "skip");
			}

			cout << "* " << std::setw(10) << student << "    " << assignment << "  " << std::setw(9) << HashToGrade << "    " << Status << endl;

			if (! Dry && (WorkToDo || Regrade))
			{
				g.Run();
			}
		}
	}
	cout << endl;

	current_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	cout << "Grading finished at: " << std::ctime(&current_time);

	cout << endl << endl;
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
