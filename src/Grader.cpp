
#include "Grader.hpp"
#include "FileSystem.hpp"
#include "Directories.hpp"
#include "Process.hpp"
#include "HTMLBuilder.hpp"
#include "IndexBuilder.hpp"

#include <chrono>


Grader::Grader(string const & student_, string const & assignment_, vector<Test> const & tests)
	: student(student_), assignment(assignment_), TestSuite(tests)
{
	StudentDirectory = AllStudentsDirectory + student + "/";
	TestsDirectory = AllTestsDirectory + assignment + "/";

	RepoDirectory = StudentDirectory + "repo/";

	StudentResultsDirectory = SiteDirectory + student + "/";
	AssignmentResultsDirectory = StudentResultsDirectory + assignment + "/";

	LogFile.open(StudentResultsDirectory + "logfile", std::ios_base::app);
	LogFile << "################################################################################" << endl;
	LogFile << endl;
	LogFile << "Grading assignment '" << assignment << "' for student '" << student << "'" << endl;
	auto current_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	LogFile << "Current time: " << std::ctime(&current_time);
	LogFile << endl;
}

string Grader::GetLatestHash()
{
	fs::current_path(RepoDirectory);

	LogFile << "Checking git status" << endl;
	LogFile << "===================" << endl;
	required_command({"git", "fetch", "origin", "master", "--tags"}, LogFile);
	LogFile << endl;

	vector<string> tags = SeparateLines(required_command_output({"git", "tag", "-l"}));

	LogFile << "git tags" << endl;
	LogFile << "========" << endl;
	if (tags.size())
	{
		for (auto tag : tags)
		{
			LogFile << tag << endl;
		}
	}
	else
	{
		LogFile << "No tags." << endl;
	}
	LogFile << endl;

	for (auto tag : tags)
	{
		if (tag == assignment)
		{
			LogFile << "Tag found for assignment found: " << tag << endl;
			return TrimWhitespace(required_command_output({"git", "rev-parse", "--short=7", tag}));
		}
	}

	LogFile << "Tag not found for assignment (" << assignment << "), grading master" << endl;
	return TrimWhitespace(required_command_output({"git", "rev-parse", "--short=7", "origin/master"}));
}

bool Grader::CheckWorkToDo(string const & hash)
{
	CurrentHash = hash;

	ResultsDirectory = AssignmentResultsDirectory + CurrentHash + "/";

	LogFile << "Creating directory for output: '" << ResultsDirectory << "'" << endl;
	fs::create_directories(ResultsDirectory);
	if (fs::is_regular_file(ResultsDirectory + "status"))
	{
		LogFile << "Grading already completed for this assignment/commit pair: " << assignment << "/" << CurrentHash << endl;
		return false;
	}

	return true;
}

void Grader::Run()
{
	if (GradeAssignment())
	{
		WriteReports();
	}
}

bool Grader::GradeAssignment()
{
	bool MakeReports = true;

	try
	{
		fs::current_path(RepoDirectory);

		DoGitUpdate();
		WriteStatusFiles();
		RunBuild();
		if (RunTests())
		{
			WritePassFile();
		}
		else
		{
			WriteToFile(ResultsDirectory + "status", "test_failure");
			WriteToFile(AssignmentResultsDirectory + "list", CurrentHash + " test_failure\n", true);
		}
	}
	catch (build_exception const & e)
	{
		LogFile << "Build failure." << endl;
		LogFile << e.what() << endl;
		WriteToFile(ResultsDirectory + "status", "build_failure");
		WriteToFile(AssignmentResultsDirectory + "list", CurrentHash + " build_failure\n", true);
	}
	catch (std::runtime_error const & e)
	{
		LogFile << "Unexpected exception occurred during grading." << endl;
		LogFile << e.what() << endl;
		MakeReports = false;
	}

	return MakeReports;
}

void Grader::WriteReports()
{
	try
	{
		fs::current_path(ResultsDirectory);
		HTMLBuilder hb(student, assignment);
		hb.Generate();
		LogFile << "Report created at: " << ResultsDirectory + "report.html" << endl;

		fs::current_path(AssignmentResultsDirectory);
		IndexBuilder ib(student, assignment, RepoDirectory);
		ib.GenerateAssignmentIndex();
		LogFile << "Assignment index created at: " << AssignmentResultsDirectory + "index.html" << endl;

		fs::current_path(SiteDirectory + student);
		ib.GenerateStudentIndex();
		LogFile << "Student index created at: " << fs::current_path().string() + "/index.html" << endl;

		fs::current_path(SiteDirectory);
		ib.GenerateCompleteIndex();
		LogFile << "Complete index created at: " << fs::current_path().string() + "/index.html" << endl;
	}
	catch (std::runtime_error const & e)
	{
		cerr << "Failed to create html report." << endl;
		cerr << "Readon: " << e.what() << endl;
	}
}

void Grader::WriteStatusFiles()
{
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
				LogFile << "Found CMakeLists.txt, doing CMake build." << endl;
				break;
			}
			else if (p.filename() == "Makefile")
			{
				BuildType = EBuildType::Make;
				LogFile << "Found Makefile, doing Make build." << endl;
			}
		}
	}

	if (BuildType == EBuildType::Auto)
	{
		if (fs::is_directory("src/"))
		{
			fs::current_path("src/");

			for(auto d : fs::directory_iterator("."))
			{
				auto p = d.path();
				if (p.has_filename())
				{
					if (p.filename() == "Makefile")
					{
						BuildType = EBuildType::Make;
						LogFile << "Found Makefile, doing Make build." << endl;
						break;
					}
				}
			}
		}
		if (BuildType == EBuildType::Auto)
		{
			LogFile << "Found no build files, attempting g++ build." << endl;
		}
	}
	LogFile << endl;


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

		vector<fs::path> cppfiles = FindAllWithExtension(".", "cpp");

		if (! cppfiles.size())
		{
			throw build_exception("no *.cpp files found to build.");
		}

		for (auto path : cppfiles)
		{
			LogFile << "Found .cpp file to build: " << path << endl;
			Args.push_back(path.string());
		}
		Args.push_back("-O3");
		Args.push_back("-o");
		Args.push_back("raytrace");

		LogFile << "- Args are:";
		for (string const & Arg : Args)
		{
			LogFile << " '" << Arg << "'";
		}
		LogFile << endl;

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

	LogFile << "Running Tests" << endl;
	LogFile << "=============" << endl;
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
	LogFile << endl;

	WriteToFile(ResultsDirectory + "tests_index", TestIndex.str());
	WriteToFile(ResultsDirectory + "image_index", ImageIndex.str());

	if (AllTestsPassed)
	{
		LogFile << "All tests passed!" << endl;
	}
	else
	{
		LogFile << "Some tests failed." << endl;
	}

	return AllTestsPassed;
}

void Grader::WritePassFile()
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
				required_command({"cp", p.string(), "./"}, LogFile);
				required_command({"cp", p.string(), "build/"}, LogFile);
				required_command({"cp", p.string(), "resources/"}, LogFile);
			}
		}
	}
}

void Grader::DoGitUpdate()
{
	if (CurrentHash == "")
	{
		throw std::runtime_error("No hash specified to run");
	}

	fs::current_path(RepoDirectory);

	LogFile << "Running git update" << endl;
	LogFile << "==================" << endl;
	LogFile << "Cleaning and resetting to " << CurrentHash << endl;
	required_command({"git", "clean", "-d", "-x", "-f"}, LogFile);
	required_command({"git", "reset", "--hard", CurrentHash}, LogFile);
	LogFile << endl;
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
		LogFile << "Found single directory (" << directory << "), entering..." << endl;
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

	LogFile << "Running test '" << TestName << "'" << endl;

	if (test.Required)
	{
		WriteToFile(RequiredFile, "");
	}
	WriteToFile(TimeoutFile, FloatToString(test.Timeout));

	vector<string> Args = Explode(ReadAsString(ArgsFile), ' ');
	Args.insert(Args.begin(), "raytrace");
	Args.insert(Args.begin(), "unbuffer");
	LogFile << "- Args are:";
	for (string & Arg : Args)
	{
		Arg = TrimWhitespace(Arg);
		LogFile << " '" << Arg << "'";
	}
	LogFile << endl;
	required_command({"cp", ArgsFile, MyArgsFile}, LogFile);

	if (fs::is_regular_file("output.png"))
	{
		fs::remove("output.png");
	}

	float CommandDuration = 0;
	ECommandStatus const CommandStatus = try_command_redirect_timeout(Args, MyOutFile, Timeout, CommandDuration);
	WriteToFile(MyDurationFile, FloatToString(CommandDuration));

	if (CommandStatus == ECommandStatus::Timeout)
	{
		LogFile << "- Timeout occurred." << endl;
		TestStatus = ETestStatus::Timeout;
	}
	else if (CommandStatus == ECommandStatus::Failure)
	{
		LogFile << "- Test failed." << endl;
	}
	else if (CommandStatus == ECommandStatus::Success)
	{
		LogFile << "- Test executed." << endl;
	}

	if (test.Type == ETestType::Text)
	{
		bool DiffSucess = try_command_redirect({"diff", "-Bw", OutFile, MyOutFile}, MyDiffFile);

		if (DiffSucess)
		{
			LogFile << "- Diff passed." << endl;
		}
		else
		{
			LogFile << "- Diff failed." << endl;
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
			required_command({"mv", "output.png", MyImageFile}, LogFile);
			required_command({"cp", ImageFile , ResultsDirectory + TestName + ".png"}, LogFile);
			string img_diff = command_output({"compare", "-metric", "AE", "-fuzz", "5%", ImageFile, MyImageFile, MyImageDiffFile});
			img_diff = TrimWhitespace(img_diff);
			LogFile << "- Image diff: " << img_diff << endl;

			int PixelDifferences = -1;
			try
			{
				PixelDifferences = std::stoi(img_diff);
			}
			catch (std::invalid_argument const & e)
			{
				LogFile << "- Image comparison failed, image size mismatch likely." << endl;
			}
			WriteToFile(MyImagePixelsFile, std::to_string(PixelDifferences));

			if (CommandStatus == ECommandStatus::Success && PixelDifferences < 1000 && PixelDifferences >= 0)
			{
				TestStatus = ETestStatus::Pass;
			}
		}
		else
		{
			LogFile << "- No image produced." << endl;
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