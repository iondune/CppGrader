
#include "Util.hpp"
#include "Test.hpp"


enum class EBuildType
{
	CMake,
	Make,
	Auto
};

enum class ETestStatus
{
	Timeout,
	Failure,
	Pass
};

class build_exception : public std::runtime_error
{

public:

	build_exception(string const & what)
		: std::runtime_error(what)
	{}

};

class Grader
{

public:

	Grader(string const & student_, string const & assignment_, vector<Test> const & tests);

	string GetLatestHash();
	bool CheckWorkToDo(string const & hash);
	void Run();

protected:

	string const student;
	string const assignment;
	vector<Test> const TestSuite;
	string CurrentHash;

	string StudentDirectory;
	string RepoDirectory;
	string ResultsDirectory;
	string StudentResultsDirectory;
	string AssignmentResultsDirectory;
	string TestsDirectory;

	std::ofstream LogFile;

	void GradeAssignment();
	void WriteReports();

	void WriteStatusFiles();
	void RunBuild();
	bool RunTests();
	void WritePassFile();

	void CopyInputFiles();
	void DoGitUpdate();
	void CheckForSingleDirectory();
	ETestStatus DoTest(Test const & test);

};
