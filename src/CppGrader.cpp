
#define SUBPROCESS_HPP_IMPLEMENTATION

#include "Util.hpp"
#include "Process.hpp"
#include "FileSystem.hpp"
#include "Directories.hpp"
#include "Test.hpp"
#include "Grader.hpp"

#include <json.hpp>



int main()
{
	string assignment = "p4";
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

	{
		Grader g("idunn01", assignment, TestSuite);
		g.Run();
	}
	return 0;

	fs::current_path(AllStudentsDirectory);

	vector<string> students = ReadAsLines("list");

	cout << "Students" << endl;
	cout << "========" << endl;
	for (auto student : students)
	{
		cout << student << endl;
	}
	cout << endl;

	for (auto student : students)
	{
		Grader g(student, assignment, TestSuite);
		g.Run();
	}

	return 0;
}

