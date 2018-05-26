
#include "HTMLBuilder.hpp"
#include "FileSystem.hpp"
#include "Process.hpp"


HTMLBuilder::HTMLBuilder(string const & student, string const & assignment, string const & hash)
{
	this->Student = student;
	this->Assignment = assignment;
	this->Hash = hash;

	File.open("report.html");
}

void HTMLBuilder::Generate()
{
	Status = ReadTrimmed("status");

	HeaderInfo();

	File << "<h2>Build Results</h2>" << endl;
	if (BuildInfo())
	{
		File << "<h2>Test Results</h2>" << endl;
		TextTests();

		File << "<h2>Image Tests</h2>" << endl;
		ImageTests();

		File << "<h2>Additional Images</h2>" << endl;
		AdditionalImages();
	}

	Cleanup();
}

void HTMLBuilder::HeaderInfo()
{
	Cat(TemplateDirectory + "top1.html", File);
	File << "<title>[" << Student << "] CSC 473 Grade Results</title>" << endl;
	Cat(TemplateDirectory + "top2.html", File);
	File << "<h1>[CSC 473] Program (" << Assignment << ") Grade Results</h1>" << endl;

	File << "<ul class=\"breadcrumb\">" << endl;
	File << "<li><a href=\"../../\">Home</a></li>" << endl;
	File << "<li><a href=\"../\">Program (" << Assignment << ")</a></li>" << endl;
	File << "<li class=\"active\">Commit (" << Hash << ")</li>" << endl;
	File << "</ul>" << endl;

	File << "<p><strong>Student:</strong> " << Student << "</p>" << endl;

	File << "<p><strong>Grade Time/Date:</strong> " << EscapeHTML(ReadTrimmed("last_run")) << "</p>" << endl;

	File << "<p><strong>Current Commit:</strong> " << Hash << "</p>" << endl;
	File << "<pre><code>";
	File << EscapeHTML(ReadTrimmed("current_commit")) << endl;
	File << "</code></pre>" << endl;

	ModalWindowStart("file_view", "Directory Structure", "primary");
	File << "<pre><code>";
	File << EscapeHTML(ReadTrimmed("directory_listing"));
	File << "</code></pre>" << endl;
	ModalWindowEnd();

	string const repo = ReadAsString(AllStudentsDirectory + Student + "/" + "link");
	File << "<a class=\"btn btn-info btn-sm\" href=\"" << repo << "\">" << "Repository" << "</a>" << endl;
	File << "<a class=\"btn btn-info btn-sm\" href=\"" << repo << "/commit/" << Hash << "\">" << "Commit" << "</a>" << endl;
}

bool HTMLBuilder::BuildInfo()
{
	bool BuildPassed = false;

	if (Status == "build_failure")
	{
		File << "<p><span class=\"text-danger\">Build failed.</span></p>" << endl;

		if (fs::is_regular_file("cmake_output"))
		{
			File << "<p>CMake output:</p>" << endl;
			File << "<pre><code>";
			File << EscapeHTML(ReadTrimmed("cmake_output"));
			File << "</code></pre>" << endl;
		}

		if (fs::is_regular_file("make_output"))
		{
			File << "<p>Make output:</p>" << endl;
			File << "<pre><code>";
			File << EscapeHTML(ReadTrimmed("make_output"));
			File << "</code></pre>" << endl;
		}

		if (fs::is_regular_file("gcc_output"))
		{
			File << "<p>gcc output:</p>" << endl;
			File << "<pre><code>";
			File << EscapeHTML(ReadTrimmed("gcc_output"));
			File << "</code></pre>" << endl;
		}
	}
	else
	{
		BuildPassed = true;
		File << "<p><span class=\"text-success\">Build succeeded.</span></p>" << endl;

		if (fs::is_regular_file("cmake_output"))
		{
			ModalWindowStart("cmake_output", "CMake Output", "primary");
			File << "<pre><code>";
			File << EscapeHTML(ReadTrimmed("cmake_output"));
			File << "</code></pre>" << endl;
			ModalWindowEnd();
		}

		if (fs::is_regular_file("make_output"))
		{
			ModalWindowStart("make_output", "Make Output", "primary");
			File << "<pre><code>";
			File << EscapeHTML(ReadTrimmed("make_output"));
			File << "</code></pre>" << endl;
			ModalWindowEnd();
		}

		if (fs::is_regular_file("gcc_output"))
		{
			ModalWindowStart("gcc_output", "gcc Output", "primary");
			File << "<pre><code>";
			File << EscapeHTML(ReadTrimmed("gcc_output"));
			File << "</code></pre>" << endl;
			ModalWindowEnd();
		}
	}

	return BuildPassed;
}

void HTMLBuilder::TextTests()
{
	vector<string> Tests = ReadAsLines("tests_index");

	if (! Tests.size())
		return;

	File << "<table class=\"table table-striped table-bordered\" style=\"width: auto;\">" << endl;
	File << "<thead>" << endl;
	File << "<tr>" << endl;
	File << "<th>Test</th>" << endl;
	File << "<th>Status</th>" << endl;
	File << "<th>Arguments</th>" << endl;
	File << "</tr>" << endl;
	File << "</thead>" << endl;
	File << "<tbody>" << endl;

	for (auto test : Tests)
	{
		File << "<tr>" << endl << "<td>" << test << "</td>" << endl;

		string const test_status = ReadTrimmed("my"s + test + ".status");

		File << "<td>" << endl;
		if (test_status == "pass")
		{
			File << "<span class=\"label label-success\">Passed</span>";
		}
		else if (test_status == "timeout")
		{
			File << "<span class=\"label label-danger\">Timeout</span>";
		}
		else if (test_status == "failure")
		{
			ModalWindowStart("diff_"s + test, "Failed - Diff Results ("s + test + ")", "danger");
			File << "<pre><code>";
			File << EscapeHTML(ReadTrimmed("my"s + test + ".diff"));
			File << "</code></pre>";
			ModalWindowEnd();
		}
		File << "</td>" << endl;

		File << "<td>";
		if (fs::is_regular_file("my"s + test + ".args"))
		{
			File << "<pre><code>" << ReadTrimmed("my"s + test + ".args") << "</code></pre>";
		}
		File << "</td>" << endl;

		File << "</tr>" << endl;
	}

	File << "</tbody></table>" << endl;
}

void HTMLBuilder::ImageTests()
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
		string const SignalFile = "my"s + test + ".signal";

		string const test_status = ReadTrimmed("my"s + test + ".status");
		string type = "danger";

		if (test_status == "pass")
		{
			File << "<span class=\"label label-success\">Passed</span>" << endl;
			type = "primary";
		}
		else if (test_status == "timeout")
		{
			File << "<span class=\"label label-warning\">Timeout</span>" << endl;
		}
		else if (test_status == "failure")
		{
			File << "<span class=\"label label-danger\">Failure</span>" << endl;
		}

		if (! fs::is_regular_file(test + ".required"))
		{
			File << "<span class=\"label label-info\">Optional</span>" << endl;
		}

		File << "</td><td>" << endl;

		File << "<p>Test took " << ReadTrimmed("my"s + test + ".duration") << "s, timeout was " << ReadTrimmed(test + ".timeout") << "s.</p>" << endl;
		if (fs::is_regular_file("my"s + test + ".args"))
		{
			File << "<p>Arguments:</p><p><pre><code>" << ReadTrimmed("my"s + test + ".args") << "</code></pre></p>" << endl;
		}

		if (fs::is_regular_file(SignalFile))
		{
			File << "<p><span class=\"text-danger\">Process terminated by signal.</span></p>" << endl;
		}

		if (! fs::is_regular_file(ImageFile))
		{
			File << "<p><span class=\"text-danger\">Image for " << test << " failed - no image produced.</span></p>" << endl;
		}
		else
		{
			File << "<p><span class=\"text-" << type << "\">Found " << ReadTrimmed("my"s + test + ".pixels") << " pixel differences - up to 1000 are allowed.</span></p>" << endl;

			ModalWindowStart("image_"s + test, "Image Comparison ("s + test + ".pov)", type);

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

			ModalWindowEnd();
		}

		ModalWindowStart("output_"s + test, "Program output ("s + test + ".pov)", type);
		File << "<pre><code>";
		File << EscapeHTML(ReadTrimmed("my"s + test + ".out"));
		File << "</code></pre>" << endl;
		ModalWindowEnd();

		File << "</td></tr>" << endl;
	}

	File << "</tbody></table>" << endl;
}

void HTMLBuilder::AdditionalImages()
{
	vector<string> Tests = ReadAsLines("additional_index");

	if (! Tests.size())
		return;

	File << "<table class=\"table table-striped table-bordered\" style=\"width: auto;\">" << endl;
	File << "<thead>" << endl;
	File << "<tr>" << endl;
	File << "<th>File</th>" << endl;
	File << "<th>Results</th>" << endl;
	File << "</tr>" << endl;
	File << "</thead>" << endl;
	File << "<tbody>" << endl;

	for (auto test : Tests)
	{
		File << "<tr><td>" << test << "</td><td>" << endl;

		string const ImageFile = "my"s + test + ".png";
		if (! fs::is_regular_file(ImageFile))
		{
			File << "<p><span class=\"text-danger\">Image for " << test << ".pov failed - no image produced.</span></p>" << endl;
		}
		else
		{
			File << "<img src=\"my" << test << ".png" << "\"             alt=\"rendered\"   id=\"" << test << "_image1\" class=\"image-toggle\" />" << endl;
		}

		File << "</td></tr>" << endl;
	}

	File << "</tbody></table>" << endl;
}

void HTMLBuilder::Cleanup()
{
	Cat(TemplateDirectory + "bottom.html", File);
}

void HTMLBuilder::CollapseButton(string const & id)
{
	File << "<button class=\"btn btn-primary\" type=\"button\" data-toggle=\"collapse\" data-target=\"#" << id << "\" aria-expanded=\"false\" aria-controls=\"" << id << "\">" << endl;
	File << "Show/Hide" << endl;
	File << "</button>" << endl;
}

void HTMLBuilder::ModalWindowStart(string const & id, string const & button_label, string const & btn_class)
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

void HTMLBuilder::ModalWindowEnd()
{
	File << "</div>" << endl;
	File << "<div class=\"modal-footer\">" << endl;
	File << "<button type=\"button\" class=\"btn btn-default\" data-dismiss=\"modal\">Close</button>" << endl;
	File << "</div>" << endl;
	File << "</div>" << endl;
	File << "</div>" << endl;
	File << "</div>" << endl;
}
