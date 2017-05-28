
#include "HTMLBuilder.hpp"
#include "FileSystem.hpp"


HTMLBuilder::HTMLBuilder(string const & student, string const & assignment)
{
	File.open("report.html");
	this->Student = student;
	this->Assignment = assignment;
}

void HTMLBuilder::Generate()
{
	status = ReadTrimmed("status");

	header_info(Student, Assignment);

	if (build_info())
	{
		File << "<h2>Test Results</h2>" << endl;

		text_tests();
	}


	cleanup();
}

void HTMLBuilder::header_info(string const & student, string const & assignment)
{
	Cat(TemplateDirectory + "top1.html", File);
	File << "<title>[" << student << "] CPE 473 Grade Results</title>" << endl;
	Cat(TemplateDirectory + "top2.html", File);
	File << "<h1>[CPE 473] Program (" << assignment << ") Grade Results</h1>" << endl;

	File << "<p>Student: " << student << "</p>" << endl;

	File << "<p>Grade Time/Date: " << ReadTrimmed("last_run") << "</p>" << endl;
	File << "<p><a href=\"../\">&lt;&lt; Back to All Grades</a></p>" << endl;

	File << "<p><span>Current Commit:</span></p>" << endl;
	File << "<pre><code>";
	File << ReadTrimmed("current_commit") << endl;
	File << "</code></pre>" << endl;

	modal_window_start("file_view", "Directory Structure", "primary");
	File << "<pre><code>";
	File << ReadTrimmed("directory_listing");
	File << "</code></pre>" << endl;
	modal_window_end();
}

bool HTMLBuilder::build_info()
{
	bool BuildPassed = false;

	if (status == "build_failure")
	{
		File << "<p><span class=\"text-danger\">Build failed.</span></p>" << endl;

		if (fs::is_regular_file("cmake_output"))
		{
			File << "<p>CMake output:</p>" << endl;
			File << "<pre><code>";
			File << ReadTrimmed("cmake_output");
			File << "</code></pre>" << endl;
		}

		if (fs::is_regular_file("make_output"))
		{
			File << "<p>Make output:</p>" << endl;
			File << "<pre><code>";
			File << ReadTrimmed("make_output");
			File << "</code></pre>" << endl;
		}
	}
	else
	{
		BuildPassed = true;
		File << "<p><span class=\"text-success\">Build succeeded.</span></p>" << endl;

		if (fs::is_regular_file("cmake_output"))
		{
			modal_window_start("cmake_output", "CMake Output", "primary");
			File << "<pre><code>";
			File << ReadTrimmed("cmake_output");
			File << "</code></pre>" << endl;
			modal_window_end();
		}

		if (fs::is_regular_file("make_output"))
		{
			modal_window_start("cmake_output", "Make Output", "primary");
			File << "<pre><code>";
			File << ReadTrimmed("make_output");
			File << "</code></pre>" << endl;
			modal_window_end();
		}
	}

	return BuildPassed;
}

void HTMLBuilder::text_tests()
{
	vector<string> Tests = ReadAsLines("tests_index");

	if (! Tests.size())
		return;

	File << "<table class=\"table table-striped table-bordered\" style=\"width: auto;\">" << endl;
	File << "<thead>" << endl;
	File << "<tr>" << endl;
	File << "<th>Test</th>" << endl;
	File << "<th>Status</th>" << endl;
	File << "</tr>" << endl;
	File << "</thead>" << endl;
	File << "<tbody>" << endl;

	for (auto test : Tests)
	{
		File << "<tr><td>" << test << "</td><td>" << endl;

		string const status = ReadTrimmed("my"s + test + ".status");

		if (status == "pass")
		{
			File << "<span class=\"label label-success\">Passed</span>" << endl;
		}
		else if (status == "timeout")
		{
			File << "<span class=\"label label-danger\">Timeout</span>" << endl;
		}
		else if (status == "failure")
		{
			modal_window_start("diff_"s + test, "Failed - Diff Results ("s + test + ")", "danger");
			File << "<pre><code>";
			File << ReadTrimmed("my"s + test + ".diff");
			File << "</code></pre>" << endl;
			modal_window_end();
		}
	}

	File << "</tbody></table>" << endl;
}

void HTMLBuilder::cleanup()
{
	Cat(TemplateDirectory + "bottom.html", File);
	// required_command({"mv", "report.html", HTMLDirectory + "index.html"});
}

void HTMLBuilder::collapse_button(string const & id)
{
	File << "<button class=\"btn btn-primary\" type=\"button\" data-toggle=\"collapse\" data-target=\"#" << id << "\" aria-expanded=\"false\" aria-controls=\"" << id << "\">" << endl;
	File << "Show/Hide" << endl;
	File << "</button>" << endl;
}

void HTMLBuilder::modal_window_start(string const & id, string const & button_label, string const & btn_class)
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

void HTMLBuilder::modal_window_end()
{
	File << "</div>" << endl;
	File << "<div class=\"modal-footer\">" << endl;
	File << "<button type=\"button\" class=\"btn btn-default\" data-dismiss=\"modal\">Close</button>" << endl;
	File << "</div>" << endl;
	File << "</div>" << endl;
	File << "</div>" << endl;
	File << "</div>" << endl;
}
