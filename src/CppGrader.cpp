
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <experimental/filesystem>
#include <subprocess.hpp>

using namespace std;
namespace fs = std::experimental::filesystem;
namespace sp = subprocess;



vector<string> SeparateLines(string const & str)
{
	vector<string> Lines;
	istringstream Stream(str);
	string Line;

	while (getline(Stream, Line))
	{
		Lines.push_back(move(Line));
	}

	return Lines;
}

string ReadAsString(string const & FileName)
{
	std::ifstream FileHandle(FileName);
	std::string String;

	FileHandle.seekg(0, std::ios::end);
	String.reserve((uint) FileHandle.tellg());
	FileHandle.seekg(0, std::ios::beg);

	String.assign((std::istreambuf_iterator<char>(FileHandle)), std::istreambuf_iterator<char>());

	return String;
}

vector<string> ReadAsLines(string const & FileName)
{
	return SeparateLines(ReadAsString(FileName));
}

string Cat(string const & FileName, ostream & File)
{
	File << ReadAsString(FileName);
}

template <typename... Args>
void required_command(std::initializer_list<string> const & cmd, Args&&... args)
{
	auto p = sp::Popen(cmd, sp::output{sp::PIPE}, sp::error{sp::STDOUT}, std::forward<Args>(args)...);
	auto res = p.communicate();
	auto retcode = p.poll();
	if (retcode > 0)
	{
		throw sp::CalledProcessError("Command failed: Non zero retcode");
	}
	cout << res.first.buf.data();
}

template <typename... Args>
string required_command_output(std::initializer_list<string> const & cmd, Args&&... args)
{
	auto p = sp::Popen(cmd, sp::output{sp::PIPE},  sp::error{sp::STDOUT}, std::forward<Args>(args)...);
	auto res = p.communicate();
	auto retcode = p.poll();
	if (retcode > 0)
	{
		throw sp::CalledProcessError("Command failed: Non zero retcode");
	}
	return res.first.buf.data();
}


string const ExecDirectory = "/home/ian";
string const StudentsDirectory = "/home/ian/students";
string const SiteDirectory = "/var/www/html/grades";
string const TemplateDirectory = "/home/ian/csc473-gradeserver/html";
// string const StudentHTMLDirectory="${site_directory}/${student}/${assignment}/"

class HTMLBuilder
{

public:

	HTMLBuilder(ostream & file)
		: File(file)
	{}

	void header_info(string const & student, string const & assignment)
	{
		Cat(TemplateDirectory + "/top1.html", File);
		cout << "<title>[" << student << "] CPE 473 Grade Results</title>" << endl;
		Cat(TemplateDirectory + "/top2.html", File);
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
		// required_command({"tree", "--filelimit", "32"});
		cout << required_command_output({"tree", "--filelimit", "32"});
		cout << "</code></pre>" << endl;
		modal_window_end();
	}

	void cleanup(string const & HTMLDirectory)
	{
		Cat(HTMLDirectory + "/bottom.html", File);
		required_command({"mv", HTMLDirectory + "/temp.html", HTMLDirectory + "/index.html"});
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

void DoGitUpdate(string const & assignment)
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
}

bool IsHidden(fs::path const & p)
{
	fs::path::string_type const name = p.filename();
	return (
		name != ".." &&
		name != "." &&
		name.size() > 0 &&
		name[0] == '.'
	);
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

struct EBuildType
{
	enum
	{
		CMake,
		Make,
		Auto
	};
};

void GradeStudent(string const & student, string const & assignment)
{
	fs::current_path(StudentsDirectory + "/" + student);

	cout << "Current path: " << fs::current_path() << endl;
	cout << endl;

	fs::current_path("repo");
	// DoGitUpdate(assignment);

	// auto build_environment = sp::environment({
	// 	{ "GLM_INCLUDE_DIR", "/usr/include/glm/" },
	// 	{ "EIGEN3_INCLUDE_DIR", "/usr/include/eigen3/" }
	// });

	CheckForSingleDirectory();

	int BuildType = EBuildType::Auto;

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

	// HTMLBuilder hb(cout);
	// hb.header_info(student, assignment);
	// hb.directory_listing();
		required_command({"tree"});

}

int main()
{
	string assignment = "p4";

	GradeStudent("idunn01", assignment);
	return 0;

	fs::current_path(StudentsDirectory);

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
		GradeStudent(student, assignment);
	}

	return 0;
}

