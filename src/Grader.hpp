
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

protected:

	string const student;
	string const assignment;
	vector<Test> const TestSuite;

	string StudentDirectory;
	string RepoDirectory;
	string ResultsDirectory;
	string TestsDirectory;

	void RunGit();
	void RunBuild();
	bool RunTests();

	void CopyInputFiles();
	static string DoGitUpdate(string const & assignment);
	static void CheckForSingleDirectory();
	ETestStatus DoTest(Test const & test);

};
