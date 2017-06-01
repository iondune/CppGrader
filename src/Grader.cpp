
#include "Grader.hpp"
#include "FileSystem.hpp"
#include "Directories.hpp"
#include "Process.hpp"
#include "HTMLBuilder.hpp"
#include "IndexBuilder.hpp"


Grader::Grader(string const & student_, string const & assignment_, vector<Test> const & tests)
	: student(student_), assignment(assignment_), TestSuite(tests)
{
	StudentDirectory = AllStudentsDirectory + student + "/";
	TestsDirectory = AllTestsDirectory + assignment + "/";

	RepoDirectory = StudentDirectory + "repo/";

	AssignmentResultsDirectory = SiteDirectory + student + "/" + assignment + "/";
}

void Grader::Run()
{
	try
	{
		fs::current_path(RepoDirectory);

		RunGit();
		RunBuild();
		if (RunTests())
		{
			WriteToFile(ResultsDirectory + "status", "passed");
			WriteToFile(AssignmentResultsDirectory + "list", CurrentHash + " passed\n", true);
			std::stringstream s;
			s << required_command_output({"date"}, sp::environment(std::map<string, string>({{"TZ", "America/Los_Angeles"}})));
			s << endl;
			s << required_command_output({"git", "log", "-n", "1", "--date=local", "HEAD"});
			s << endl;
			WriteToFile(ResultsDirectory + "passfile", s.str());
		}
		else
		{
			WriteToFile(ResultsDirectory + "status", "test_failure");
			WriteToFile(AssignmentResultsDirectory + "list", CurrentHash + " test_failure\n", true);
		}
	}
	catch (skip_exception const & e)
	{}
	catch (build_exception const & e)
	{
		cout << "Build failure." << endl;
		cout << e.what() << endl;
		WriteToFile(ResultsDirectory + "status", "build_failure");
		WriteToFile(AssignmentResultsDirectory + "list", CurrentHash + " build_failure\n", true);
	}
	catch (std::runtime_error const & e)
	{
		cerr << "Unexpected exception occurred during grading." << endl;
		cerr << e.what() << endl;
	}

	try
	{
		fs::current_path(ResultsDirectory);
		HTMLBuilder hb(student, assignment);
		hb.Generate();
		cout << "Report created at: " << ResultsDirectory + "report.html" << endl;

		fs::current_path(AssignmentResultsDirectory);
		IndexBuilder ib(student, assignment, RepoDirectory);
		ib.GenerateAssignmentIndex();
		cout << "Assignment index created at: " << AssignmentResultsDirectory + "index.html" << endl;

		fs::current_path(SiteDirectory + student);
		ib.GenerateStudentIndex();
		cout << "Student index created at: " << fs::current_path().string() + "/index.html" << endl;

		fs::current_path(SiteDirectory);
		ib.GenerateCompleteIndex();
		cout << "Complete index created at: " << fs::current_path().string() + "/index.html" << endl;
	}
	catch (std::runtime_error const & e)
	{
		cerr << "Failed to create html report." << endl;
		cerr << "Readon: " << e.what() << endl;
	}
}


void Grader::RunGit()
{
	CurrentHash = DoGitUpdate(assignment);

	ResultsDirectory = AssignmentResultsDirectory + CurrentHash + "/";

	cout << "Creating directory for output: '" << ResultsDirectory << "'" << endl;
	fs::create_directories(ResultsDirectory);
	if (fs::is_regular_file(ResultsDirectory + "status"))
	{
		cout << "Grading already completed for this assignment/commit pair: " << assignment << "/" << CurrentHash << endl;
		if (Regrade)
		{
			cout << "But --regrade flag specified, grading again." << endl;
		}
		else
		{
			throw skip_exception("Already graded.");
		}
	}

	try_command_redirect({"date"}, ResultsDirectory + "last_run", sp::environment(std::map<string, string>({{"TZ", "America/Los_Angeles"}})));
	try_command_redirect({"tree", "--filelimit", "32"}, ResultsDirectory + "directory_listing");
	try_command_redirect({"git", "log", "-n", "1", "--date=local", "HEAD"}, ResultsDirectory + "current_commit");
}

void Grader::RunBuild()
{
	CheckForSingleDirectory();
	CopyInputFiles();

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
			else if (p.filename() == "Makefile")
			{
				BuildType = EBuildType::Make;
				cout << "Found Makefile, doing Make build." << endl;
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
		fs::current_path("build");
		RemoveIfExists("CMakeCache.txt");

		if (! try_command_redirect({"cmake", ".."}, ResultsDirectory + "cmake_output", std::move(build_environment)))
		{
			throw build_exception("CMake build failed.");
		}

		if (! try_command_redirect({"make"}, ResultsDirectory + "make_output"))
		{
			throw build_exception("Make build failed.");
		}
	}
	else if (BuildType == EBuildType::Make)
	{
		if (! try_command_redirect({"make"}, ResultsDirectory + "make_output"))
		{
			throw build_exception("Make build failed.");
		}
	}
	else if (BuildType == EBuildType::Auto)
	{
		vector<string> Args;
		Args.push_back("gcc");

		vector<fs::path> cppfiles = FindAllWithExtension("src/", "cpp");

		if (! cppfiles.size())
		{
			throw build_exception("no *.cpp files found to build.");
		}

		for (auto path : cppfiles)
		{
			cout << "Found .cpp file to build: " << path << endl;
			Args.push_back(path.string());
		}
		Args.push_back("-O3");
		Args.push_back("-o");
		Args.push_back("raytrace");

		cout << "- Args are:";
		for (string const & Arg : Args)
		{
			cout << " '" << Arg << "'";
		}
		cout << endl;

		if (! try_command_redirect(Args, ResultsDirectory + "gcc_output"))
		{
			throw build_exception("gcc build failed.");
		}
	}

	if (! fs::is_regular_file("raytrace"))
	{
		throw build_exception("Executable missing.");
	}
}

bool Grader::RunTests()
{
	bool AllTestsPassed = true;

	std::stringstream TestIndex;
	std::stringstream ImageIndex;

	cout << "Running Tests" << endl;
	cout << "=============" << endl;
	for (Test const & test : TestSuite)
	{
		if (DoTest(test) != ETestStatus::Pass && test.Required)
		{
			AllTestsPassed = false;
		}

		if (test.Type == ETestType::Text)
		{
			TestIndex << test.Name << endl;
		}
		else if (test.Type == ETestType::Image)
		{
			ImageIndex << test.Name << endl;
		}
	}
	cout << endl;

	WriteToFile(ResultsDirectory + "tests_index", TestIndex.str());
	WriteToFile(ResultsDirectory + "image_index", ImageIndex.str());

	if (AllTestsPassed)
	{
		cout << "All tests passed!" << endl;
	}
	else
	{
		cout << "Some tests failed." << endl;
	}

	return AllTestsPassed;
}


/////////////
// Helpers //
/////////////

void Grader::CopyInputFiles()
{
	fs::create_directories("build/");
	fs::create_directories("resources/");

	for (auto d : fs::directory_iterator(InputsDirectory))
	{
		auto p = d.path();

		if (! IsHidden(p))
		{
			if (fs::is_regular_file(p))
			{
				required_command({"cp", p.string(), "./"});
				required_command({"cp", p.string(), "build/"});
				required_command({"cp", p.string(), "resources/"});
			}
		}
	}
}

string Grader::DoGitUpdate(string const & assignment)
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

void Grader::CheckForSingleDirectory()
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

ETestStatus Grader::DoTest(Test const & test)
{
	ETestStatus TestStatus = ETestStatus::Failure;

	string const TestName = test.Name;
	float const Timeout = test.Timeout;

	string const ArgsFile = TestsDirectory + TestName + ".args";
	string const OutFile = TestsDirectory + TestName + ".out";
	string const ImageFile = TestsDirectory + TestName + ".png";

	string const RequiredFile = ResultsDirectory + TestName + ".required";
	string const TimeoutFile = ResultsDirectory + TestName + ".timeout";

	string const MyOutFile = ResultsDirectory + "my" + TestName + ".out";
	string const MyDiffFile = ResultsDirectory + "my" + TestName + ".diff";
	string const MyImageFile = ResultsDirectory + "my" + TestName + ".png";
	string const MyImageDiffFile = ResultsDirectory + "difference_my" + TestName + ".png";
	string const MyImagePixelsFile = ResultsDirectory + "my" + TestName + ".pixels";
	string const MyStatusFile = ResultsDirectory + "my" + TestName + ".status";
	string const MyDurationFile = ResultsDirectory + "my" + TestName + ".duration";
	string const MyArgsFile = ResultsDirectory + "my" + TestName + ".args";

	cout << "Running test '" << TestName << "'" << endl;

	if (test.Required)
	{
		WriteToFile(RequiredFile, "");
	}
	WriteToFile(TimeoutFile, FloatToString(test.Timeout));

	vector<string> Args = Explode(ReadAsString(ArgsFile), ' ');
	Args.insert(Args.begin(), "raytrace");
	cout << "- Args are:";
	for (string & Arg : Args)
	{
		Arg = TrimWhitespace(Arg);
		cout << " '" << Arg << "'";
	}
	cout << endl;
	required_command({"cp", ArgsFile, MyArgsFile});

	if (fs::is_regular_file("output.png"))
	{
		fs::remove("output.png");
	}

	float CommandDuration = 0;
	ECommandStatus const CommandStatus = try_command_redirect_timeout(Args, MyOutFile, Timeout, CommandDuration);
	WriteToFile(MyDurationFile, FloatToString(CommandDuration));

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
		bool DiffSucess = try_command_redirect({"diff", "-Bw", OutFile, MyOutFile}, MyDiffFile);

		if (DiffSucess)
		{
			cout << "- Diff passed." << endl;
		}
		else
		{
			cout << "- Diff failed." << endl;
		}

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
			required_command({"cp", ImageFile , ResultsDirectory + TestName + ".png"});
			string img_diff = command_output({"compare", "-metric", "AE", "-fuzz", "5%", ImageFile, MyImageFile, MyImageDiffFile});
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