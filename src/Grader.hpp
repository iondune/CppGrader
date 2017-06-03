
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

class skip_exception : public std::runtime_error
{

public:

	skip_exception(string const & what)
		: std::runtime_error(what)
	{}

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

	void Run();

	bool Regrade = false;

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

	void RunGit();
	void RunBuild();
	bool RunTests();

	void CopyInputFiles();
	string DoGitUpdate(string const & assignment);
	void CheckForSingleDirectory();
	ETestStatus DoTest(Test const & test);

};
