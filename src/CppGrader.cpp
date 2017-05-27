
#include "Util.hpp"
#include "Process.hpp"
#include "FileSystem.hpp"
#include "Directories.hpp"
#include "Test.hpp"

#include <json.hpp>
using nlohmann::json;

using std::cout;
using std::endl;


enum class EBuildType
{
	CMake,
	Make,
	Auto
};

enum class EFailureType
{
	Skip,
	Build,
};

enum class ETestStatus
{
	Timeout,
	Failure,
	Pass
};

class grade_exception : public std::runtime_error
{

public:

	grade_exception(string const & what, EFailureType const failureType)
		: std::runtime_error(what), FailureType(failureType)
	{}

	EFailureType const FailureType;

};

class Grader
{

public:

	Grader(string const & student_, string const & assignment_, vector<Test> const & tests)
		: student(student_), assignment(assignment_), TestSuite(tests)
	{
		StudentDirectory = AllStudentsDirectory + student + "/";
		RepoDirectory = StudentDirectory + "repo/";
		TestsDirectory = AllTestsDirectory + assignment + "/";
	}

	void Run()
	{
		try
		{
			fs::current_path(RepoDirectory);

			RunGit();
			RunBuild();
			RunTests();
		}
		catch (grade_exception const & e)
		{
			cout << e.what() << endl;
		}
	}

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
	void RunTests();

	void CopyInputFiles()
	{
		fs::create_directories(RepoDirectory + "build/");
		fs::create_directories(RepoDirectory + "resources/");

		string directory;
		for (auto d : fs::directory_iterator(InputsDirectory))
		{
			auto p = d.path();

			if (! IsHidden(p))
			{
				if (fs::is_regular_file(p))
				{
					required_command({"cp", p.string(), RepoDirectory});
					required_command({"cp", p.string(), RepoDirectory + "build/"});
					required_command({"cp", p.string(), RepoDirectory + "resources/"});
				}
			}
		}
	}

	static string DoGitUpdate(string const & assignment)
	{
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
				required_command({"git", "checkout", tag});
				FoundTag = true;
			}
		}

		if (! FoundTag)
		{
			cout << "Tag not found, building master." << endl;
			required_command({"git", "checkout", "master"});
		}
		cout << endl;

		cout << "Current commit" << endl;
		cout << "==============" << endl;
		required_command({"git", "log", "-n", "1", "--date=local", "HEAD"});
		cout << endl;

		return TrimWhitespace(required_command_output({"git", "rev-parse", "--short=7", "HEAD"}));
	}

	static void CheckForSingleDirectory()
	{
		int NumFiles = 0;
		int NumDirectories = 0;

		string directory;
		for (auto d : fs::directory_iterator("."))
		{
			auto p = d.path();

			if (! IsHidden(p))
			{
				if (fs::is_directory(p))
				{
					NumDirectories += 1;
					directory = p;
				}
				else if (fs::is_regular_file(p))
				{
					NumFiles += 1;
				}
			}
		}

		if (NumFiles == 0 && NumDirectories == 1)
		{
			cout << "Found single directory (" << directory << "), entering..." << endl;
			fs::current_path(directory);
		}
	}

	ETestStatus DoTest(Test const & test)
	{
		ETestStatus TestStatus = ETestStatus::Failure;

		string const TestName = test.Name;
		float const Timeout = test.Timeout;

		string const ArgsFile = TestsDirectory + TestName + ".args";
		string const OutFile = TestsDirectory + TestName + ".out";
		string const ImageFile = TestsDirectory + TestName + ".png";

		string const MyOutFile = ResultsDirectory + "my" + TestName + ".out";
		string const MyDiffFile = ResultsDirectory + "my" + TestName + ".diff";
		string const MyImageFile = ResultsDirectory + "my" + TestName + ".png";
		string const MyImageDiffFile = ResultsDirectory + "difference_my" + TestName + ".png";
		string const MyImagePixelsFile = ResultsDirectory + "my" + TestName + ".pixels";
		string const MyStatusFile = ResultsDirectory + "my" + TestName + ".status";

		cout << "Running test '" << TestName << "'" << endl;

		vector<string> Args = Explode(ReadAsString(ArgsFile), ' ');
		Args.insert(Args.begin(), "raytrace");
		cout << "- Args are:";
		for (string & Arg : Args)
		{
			Arg = TrimWhitespace(Arg);
			cout << " '" << Arg << "'";
		}
		cout << endl;

		ECommandStatus const CommandStatus = try_command_redirect_timeout(Args, MyOutFile, Timeout);

		if (CommandStatus == ECommandStatus::Timeout)
		{
			cout << "- Timeout occurred." << endl;
			TestStatus = ETestStatus::Timeout;
		}
		else if (CommandStatus == ECommandStatus::Failure)
		{
			cout << "- Test failed." << endl;
		}
		else if (CommandStatus == ECommandStatus::Success)
		{
			cout << "- Test executed." << endl;
		}

		if (test.Type == ETestType::Text)
		{
			bool DiffSucess = try_command_redirect({"diff", OutFile, MyOutFile}, MyDiffFile);

			if (CommandStatus == ECommandStatus::Success && DiffSucess)
			{
				TestStatus = ETestStatus::Pass;
			}
		}
		else if (test.Type == ETestType::Image)
		{
			if (fs::is_regular_file("output.png"))
			{
				required_command({"mv", "output.png", MyImageFile});
				string img_diff = required_command_output({"compare", "-metric", "AE", "-fuzz", "5%", ImageFile, MyImageFile, MyImageDiffFile});
				img_diff = TrimWhitespace(img_diff);
				cout << "- Image diff: " << img_diff << endl;

				int const PixelDifferences = std::stoi(img_diff);
				WriteToFile(MyImagePixelsFile, std::to_string(PixelDifferences));

				if (CommandStatus == ECommandStatus::Success && PixelDifferences < 1000)
				{
					TestStatus = ETestStatus::Pass;
				}
			}
			else
			{
				cout << "- No image produced." << endl;
			}
		}

		switch (TestStatus)
		{
		case ETestStatus::Timeout:
			WriteToFile(MyStatusFile, "timeout");
			break;
		case ETestStatus::Failure:
			WriteToFile(MyStatusFile, "failure");
			break;
		case ETestStatus::Pass:
			WriteToFile(MyStatusFile, "pass");
			break;
		}

		return TestStatus;
	}

};

void Grader::RunGit()
{
	string const CurrentHash = DoGitUpdate(assignment);

	ResultsDirectory = StudentDirectory + "results/" + assignment + "/" + CurrentHash + "/";

	cout << "Creating directory for output: '" << ResultsDirectory << "'" << endl;
	fs::create_directories(ResultsDirectory);
	if (fs::is_regular_file(ResultsDirectory + "grading_done"))
	{
		cout << "Grading already completed for this assignment/commit pair: " << assignment << "/" << CurrentHash << endl;
		throw grade_exception("Already run.", EFailureType::Skip);
	}

	try_command_redirect({"date"}, ResultsDirectory + "last_run", sp::environment(std::map<string, string>({{"TZ", "America/Los_Angeles"}})));
	try_command_redirect({"tree", "--filelimit", "32"}, ResultsDirectory + "directory_listing");
	try_command_redirect({"git", "log", "-n", "1", "--date=local", "HEAD"}, ResultsDirectory + "current_commit");
}

void Grader::RunBuild()
{
	CheckForSingleDirectory();

	EBuildType BuildType = EBuildType::Auto;

	for(auto d : fs::directory_iterator("."))
	{
		auto p = d.path();
		if (p.has_filename())
		{
			if (p.filename() == "CMakeLists.txt")
			{
				BuildType = EBuildType::CMake;
				cout << "Found CMakeLists.txt, doing CMake build." << endl;
			}
			else if (p.filename() == "CMakeLists.txt")
			{
				BuildType = EBuildType::Make;
				cout << "Found CMakeLists.txt, doing Make build." << endl;
			}
		}
	}

	if (BuildType == EBuildType::Auto)
	{
		cout << "Found no build files, attempting g++ build." << endl;
	}
	cout << endl;


	sp::environment build_environment = sp::environment{{
		{ "GLM_INCLUDE_DIR", "/usr/include/glm/" },
		{ "EIGEN3_INCLUDE_DIR", "/usr/include/eigen3/" }
	}};

	if (BuildType == EBuildType::CMake)
	{
		fs::create_directories("build/");
		fs::current_path("build");
		RemoveIfExists("CMakeCache.txt");

		if (! try_command_redirect({"cmake", ".."}, ResultsDirectory + "cmake_output", std::move(build_environment)))
		{
			throw grade_exception("CMake build failed.", EFailureType::Build);
		}

		if (! try_command_redirect({"make"}, ResultsDirectory + "make_output"))
		{
			throw grade_exception("Make build failed.", EFailureType::Build);
		}
	}
	else if (BuildType == EBuildType::Make)
	{
		if (! try_command_redirect({"make"}, ResultsDirectory + "make_output"))
		{
			throw grade_exception("Make build failed.", EFailureType::Build);
		}
	}
	else if (BuildType == EBuildType::Auto)
	{
		throw grade_exception("gcc *.cpp auto-build not supported.", EFailureType::Build);
	}
}

void Grader::RunTests()
{
	if (! fs::is_regular_file("raytrace"))
	{
		throw grade_exception("Executable missing.", EFailureType::Build);
	}

	CopyInputFiles();


	cout << "Running Tests" << endl;
	cout << "=============" << endl;
	for (Test const & test : TestSuite)
	{
		DoTest(test);
	}
	cout << endl;
}

int main()
{
	string assignment = "p4";
	vector<Test> TestSuite;

	try
	{
		std::ifstream i(AllTestsDirectory + assignment + "/" + "tests.json");
		json j;
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

