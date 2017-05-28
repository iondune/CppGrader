
#include "HTMLBuilder.hpp"


HTMLBuilder::HTMLBuilder(string const & student, string const & assignment)
{
	File.open("report.html");
	this->Student = student;
	this->Assignment = assignment;
}

void HTMLBuilder::Generate()
{
	header_info(Student, Assignment);

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
