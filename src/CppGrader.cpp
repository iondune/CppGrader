
#include "Util.h"
#include "Process.h"
#include "FileSystem.h"

using namespace std;


string const ExecDirectory = "/home/ian/";
string const AllStudentsDirectory = "/home/ian/students/";
string const SiteDirectory = "/var/www/html/grades/";
string const TemplateDirectory = "/home/ian/csc473-gradeserver/html/";
// string const StudentHTMLDirectory="${site_directory}/${student}/${assignment}/"

class HTMLBuilder
{

public:

	HTMLBuilder(ostream & file)
		: File(file)
	{}

	void header_info(string const & student, string const & assignment)
	{
		Cat(TemplateDirectory + "top1.html", File);
		cout << "<title>[" << student << "] CPE 473 Grade Results</title>" << endl;
		Cat(TemplateDirectory + "top2.html", File);
		cout << "<h1>[CPE 473] Program (" << assignment << ") Grade Results</h1>" << endl;

		cout << "<p>Student: " << student << "</p>" << endl;

		cout << "<p>Last Run: " << required_command_output({"date"}, sp::environment(std::map<string, string>({{"TZ", "America/Los_Angeles"}}))) << "</p>" << endl;
		cout << "<p><a href=\"../\">&lt;&lt; Back to All Grades</a></p>" << endl;


		cout << "<p><span>Current Commit:</span></p>" << endl;
		cout << "<pre><code>";
		cout << required_command_output({"git", "log", "-n", "1", "--date=local", "HEAD"}) << endl;
		cout << "</code></pre>" << endl;
	}

	void directory_listing()
	{
		modal_window_start("file_view", "Directory Structure", "primary");
		cout << "<pre><code>";
		cout << required_command_output({"tree", "--filelimit", "32"});
		cout << "</code></pre>" << endl;
		modal_window_end();
	}

	void cleanup(string const & HTMLDirectory)
	{
		Cat(HTMLDirectory + "bottom.html", File);
		required_command({"mv", HTMLDirectory + "temp.html", HTMLDirectory + "index.html"});
	}

	void collapse_button(string const & id)
	{
		File << "<button class=\"btn btn-primary\" type=\"button\" data-toggle=\"collapse\" data-target=\"#" << id << "\" aria-expanded=\"false\" aria-controls=\"" << id << "\">" << endl;
		File << "Show/Hide" << endl;
		File << "</button>" << endl;
	}

	void modal_window_start(string const & id, string const & button_label, string const & btn_class)
	{
		File << "<button type=\"button\" class=\"btn btn-" << btn_class << " btn-sm\" data-toggle=\"modal\" data-target=\"#" << id << "\">" << endl;
		File << button_label << endl;
		File << "</button>" << endl;
		File << "<div class=\"modal fade\" id=\"" << id << "\" tabindex=\"-1\" role=\"dialog\">" << endl;
		File << "<div class=\"modal-dialog\" role=\"document\">" << endl;
		File << "<div class=\"modal-content\">" << endl;
		File << "<div class=\"modal-header\">" << endl;
		File << "<button type=\"button\" class=\"close\" data-dismiss=\"modal\" aria-label=\"Close\">&times;</button>" << endl;
		File << "<h4 class=\"modal-title\">" << button_label << "</h4>" << endl;
		File << "</div>" << endl;
		File << "<div class=\"modal-body\">" << endl;
	}

	void modal_window_end()
	{
		File << "</div>" << endl;
		File << "<div class=\"modal-footer\">" << endl;
		File << "<button type=\"button\" class=\"btn btn-default\" data-dismiss=\"modal\">Close</button>" << endl;
		File << "</div>" << endl;
		File << "</div>" << endl;
		File << "</div>" << endl;
		File << "</div>" << endl;
	}

	ostream & File;

};

string DoGitUpdate(string const & assignment)
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

void CheckForSingleDirectory()
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

	Grader(string const & student, string const & assignment)
	{
		this->student = student;
		this->assignment = assignment;

		StudentDirectory = AllStudentsDirectory + student + "/";
		RepoDirectory = StudentDirectory + "repo/";
	}

	void Run()
	{
		try
		{
			RunGit();
			RunBuild();
		}
		catch (grade_exception const & e)
		{
			cout << e.what() << endl;
		}
	}

protected:

	string student;
	string assignment;

	string StudentDirectory;
	string RepoDirectory;
	string ResultsDirectory;

	void RunGit();
	void RunBuild();

};

void Grader::RunGit()
{
	fs::current_path(RepoDirectory);
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

int main()
{
	string assignment = "p4";

	{
		Grader g("idunn01", assignment);
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
		Grader g(student, assignment);
		g.Run();
	}

	return 0;
}

