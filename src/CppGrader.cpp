
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


template <typename... Args>
void required_command(std::initializer_list<string> const & cmd, Args&&... args)
{
	auto p = sp::Popen(cmd, sp::output{sp::PIPE},  sp::error{sp::STDOUT}, std::forward<Args>(args)...);
	auto res = p.communicate();
	auto retcode = p.poll();
	if (retcode > 0)
	{
		throw sp::CalledProcessError("Command failed : Non zero retcode");
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
		throw sp::CalledProcessError("Command failed : Non zero retcode");
	}
	return res.first.buf.data();
}


string const ExecDirectory = "/home/ian";
string const StudentsDirectory = "/home/ian/students";
string const SiteDirectory = "/var/www/html/grades";
string const TemplateDirectory = "/home/ian/csc473-gradeserver/html";


class HTMLBuilder
{

public:

	HTMLBuilder(string const & FileName)
	{

	}

	void cleanup()
	{
		// cat "$html_directory/bottom.html" >> "$student_site"
		// mv "${student_html_directory}/temp.html" "${student_html_directory}/index.html"
	}

	void collapse_button()
	{
		// echo '<button class="btn btn-primary" type="button" data-toggle="collapse" data-target="#'$1'" aria-expanded="false" aria-controls="'$1'">' >> "$student_site"
		// echo 'Show/Hide' >> "$student_site"
		// echo '</button>' >> "$student_site"
	}

	void modal_window_start()
	{
		// echo '<button type="button" class="btn btn-'$3' btn-sm" data-toggle="modal" data-target="#'$1'">' >> "$student_site"
		// echo "$2" >> "$student_site"
		// echo '</button>' >> "$student_site"
		// echo '<div class="modal fade" id="'$1'" tabindex="-1" role="dialog">' >> "$student_site"
		// echo '<div class="modal-dialog" role="document">' >> "$student_site"
		// echo '<div class="modal-content">' >> "$student_site"
		// echo '<div class="modal-header">' >> "$student_site"
		// echo '<button type="button" class="close" data-dismiss="modal" aria-label="Close">&times;</button>' >> "$student_site"
		// echo '<h4 class="modal-title">'$2'</h4>' >> "$student_site"
		// echo '</div>' >> "$student_site"
		// echo '<div class="modal-body">' >> "$student_site"
	}

	void modal_window_end()
	{
		// echo '</div>' >> "$student_site"
		// echo '<div class="modal-footer">' >> "$student_site"
		// echo '<button type="button" class="btn btn-default" data-dismiss="modal">Close</button>' >> "$student_site"
		// echo '</div>' >> "$student_site"
		// echo '</div>' >> "$student_site"
		// echo '</div>' >> "$student_site"
		// echo '</div>' >> "$student_site"
	}

	ofstream File;

};


int main()
{
	string assignment = "p4";

	fs::current_path(StudentsDirectory + "/idunn01");

	cout << "Current path: " << fs::current_path() << endl;
	cout << endl;

	fs::current_path("repo");

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

	auto build_environment = sp::environment({
		{ "GLM_INCLUDE_DIR", "/usr/include/glm/" },
		{ "EIGEN3_INCLUDE_DIR", "/usr/include/eigen3/" }
	});

	return 0;
}

