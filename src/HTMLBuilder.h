
#pragma once

#include "Util.h"


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
