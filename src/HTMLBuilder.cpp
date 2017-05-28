
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

	File << "<h2>Build Results</h2>" << endl;
	if (build_info())
	{
		File << "<h2>Test Results</h2>" << endl;

		text_tests();

		File << "<h2>Image Tests</h2>" << endl;

		image_tests();
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

		File << "</td></tr>" << endl;
	}

	File << "</tbody></table>" << endl;
}

void HTMLBuilder::image_tests()
{
	vector<string> Tests = ReadAsLines("image_index");

	if (! Tests.size())
		return;

	File << "<table class=\"table table-striped table-bordered\" style=\"width: auto;\">" << endl;
	File << "<thead>" << endl;
	File << "<tr>" << endl;
	File << "<th>Test</th>" << endl;
	File << "<th>Status</th>" << endl;
	File << "<th>Results</th>" << endl;
	File << "</tr>" << endl;
	File << "</thead>" << endl;
	File << "<tbody>" << endl;

	for (auto test : Tests)
	{
		File << "<tr><td>" << test << "</td><td>" << endl;

		string const ImageFile = "my"s + test + ".png";

		string const status = ReadTrimmed("my"s + test + ".status");
		string type = "danger";

		if (status == "pass")
		{
			File << "<span class=\"label label-success\">Passed</span>" << endl;
			type = "primary";
		}
		else if (status == "timeout")
		{
			File << "<span class=\"label label-danger\">Timeout</span>" << endl;
		}
		else if (status == "failure")
		{
			File << "<span class=\"label label-danger\">Failure</span>" << endl;
		}

		File << "</td><td>" << endl;

		if (! fs::is_regular_file(ImageFile))
		{
			File << "<p><span class=\"text-danger\">Image for " << test << " failed - no image produced.</span></p>" << endl;
		}
		else
		{
			File << "<p><span class=\"text-" << type << "\">Found " << ReadTrimmed("my"s + test + ".pixels") << " pixel differents - up to 1000 are allowed.</span></p>" << endl;

			modal_window_start("image_"s + test, "Image Comparison ("s + test + ".pov)", type);

			File << "<div class=\"btn-group\" data-toggle=\"buttons\">" << endl;
			File << "<label class=\"btn btn-primary image-toggler\" data-test-name=\"" << test << "\" data-image-number=\"1\">" << endl;
			File << "<input type=\"radio\" name=\"options\" id=\"option1\"> Rendered" << endl;
			File << "</label>" << endl;
			File << "<label class=\"btn btn-primary image-toggler\" data-test-name=\"" << test << "\" data-image-number=\"2\">" << endl;
			File << "<input type=\"radio\" name=\"options\" id=\"option2\"> Expected" << endl;
			File << "</label>" << endl;
			File << "<label class=\"btn btn-primary image-toggler\" data-test-name=\"" << test << "\" data-image-number=\"3\">" << endl;
			File << "<input type=\"radio\" name=\"options\" id=\"option3\"> Difference" << endl;
			File << "</label>" << endl;
			File << "</div>" << endl;
			File << "<div>" << endl;
			File << "<img src=\"my" << test << ".png" << "\"             alt=\"rendered\"   id=\"" << test << "_image1\" class=\"image-toggle\" />" << endl;
			File << "<img src=\"" << test << ".png" << "\"               alt=\"expected\"   id=\"" << test << "_image2\" class=\"image-toggle\" style=\"display:none;\" />" << endl;
			File << "<img src=\"difference_my" << test << ".png" << "\"  alt=\"difference\" id=\"" << test << "_image3\" class=\"image-toggle\" style=\"display:none;\" />" << endl;
			File << "</div>" << endl;

			modal_window_end();
		}

		modal_window_start("output_"s + test, "Program output ("s + test + ".pov)", type);
		File << "<pre><code>";
		File << ReadTrimmed("my"s + test + ".out");
		File << "</code></pre>" << endl;
		modal_window_end();

		File << "</td></tr>" << endl;
	}

	File << "</tbody></table>" << endl;
}

void HTMLBuilder::cleanup()
{
	Cat(TemplateDirectory + "bottom.html", File);
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
