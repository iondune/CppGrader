
#define SUBPROCESS_HPP_IMPLEMENTATION

#include "Util.hpp"
#include "Process.hpp"
#include "FileSystem.hpp"
#include "Directories.hpp"
#include "Test.hpp"
#include "Grader.hpp"

#include <json.hpp>


void PrintUsage(string const & exec_name)
{
	cerr << "usage: " << exec_name << " all" << endl;
	cerr << "   or: " << exec_name << " [student] [assignment]" << endl;
}

void GradeStudentAssignment()
{

}

int main(int argc, char const ** argv)
{
	vector<string> Arguments;
	for (int i = 0; i < argc; ++ i)
	{
		Arguments.push_back(argv[i]);
	}

	if (Arguments.size() == 0)
	{
		PrintUsage("CppGrader");
		return 1;
	}

	if (Arguments.size() == 1)
	{
		PrintUsage(Arguments[0]);
		return 1;
	}

	if (Arguments.size() == 2)
	{
		if (Arguments[1] != "all")
		{
			PrintUsage(Arguments[0]);
			return 1;
		}

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
				cout << "Failed to parse test suite for assignment '" << assignment << "'" << endl;
				cout << e.what() << endl;
				return 1;
			}

			for (string const & student : students)
			{
				Grader g(student, assignment, TestSuite);
				g.Run();
			}
		}
		return 0;
	}

	if (Arguments.size() == 3)
	{
		string const student = Arguments[1];
		string const assignment = Arguments[2];

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
			cout << "Failed to parse test suite for assignment '" << assignment << "'" << endl;
			cout << e.what() << endl;
			return 1;
		}

		Grader g(student, assignment, TestSuite);
		g.Run();

		return 0;
	}

	PrintUsage(Arguments[0]);
	return 1;
}
