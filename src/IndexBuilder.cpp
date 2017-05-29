
#include "IndexBuilder.hpp"
#include "FileSystem.hpp"


IndexBuilder::IndexBuilder(string const & student, string const & assignment)
{
	this->Student = student;
	this->Assignment = assignment;
}

void IndexBuilder::GenerateAssignmentIndex()
{
	File.open("index.html");

	Cat(TemplateDirectory + "top1.html", File);
	File << "<title>[" << Student << "] CPE 473 Grade Results</title>" << endl;
	Cat(TemplateDirectory + "top2.html", File);

	File << "<h1>[CPE 473] Program (" << Assignment << ") Grade Results</h1>" << endl;
	File << "<p>Student: " << Student << "</p>" << endl;
	File << "<p><a href=\"../\">&lt;&lt; Back to All Programs</a></p>" << endl;

	File << "<table class=\"table table-striped table-bordered\" style=\"width: auto;\">" << endl;
	File << "<thead>" << endl;
	File << "<tr>" << endl;
	File << "<th>Commit</th>" << endl;
	File << "<th>Status</th>" << endl;
	File << "</tr>" << endl;
	File << "</thead>" << endl;
	File << "<tbody>" << endl;

	vector<string> pairs = ReadAsLines("list");
	std::reverse(pairs.begin(), pairs.end());

	for (string pair : pairs)
	{
		vector<string> split = Explode(pair, ' ');

		if (split.size() == 2)
		{
			string const hash = split[0];
			string const status = split[1];
			File << "<tr><td><a href=\"" << hash << "/report.html\">" << hash << "</a></td><td>" << endl;

			if (status == "passed")
			{
				File << "<span class=\"label label-success\">Passed</span>" << endl;
			}
			else if (status == "test_failure")
			{
				File << "<span class=\"label label-warning\">Test Failure</span>" << endl;
			}
			else if (status == "build_failure")
			{
				File << "<span class=\"label label-danger\">Build Failure</span>" << endl;
			}
			else
			{
				File << "<span class=\"label label-default\">" << status << "</span>" << endl;
			}

			File << "</td></tr>" << endl;
		}
	}

	File << "</tbody></table>" << endl;

	Cat(TemplateDirectory + "bottom.html", File);

	File.close();
}

void IndexBuilder::GenerateStudentIndex()
{
	File.open("table.html");

	File << "<p>Student: " << Student << "</p>" << endl;

	File << "<table class=\"table table-striped table-bordered\" style=\"width: auto;\">" << endl;
	File << "<thead>" << endl;
	File << "<tr>" << endl;
	File << "<th>Assignment</th>" << endl;
	File << "<th>Status</th>" << endl;
	File << "<th>Latest Graded Commit</th>" << endl;
	File << "</tr>" << endl;
	File << "</thead>" << endl;
	File << "<tbody>" << endl;

	vector<string> assignments = ReadAsLines(ExecDirectory + "assignments");

	for (string const & assignment : assignments)
	{
		File << "<tr><td><a href=\"" << assignment << "/\">" << assignment << "</a></td><td>" << endl;

		vector<string> pairs = ReadAsLines(assignment + "/" + "list");

		string hash = "none";
		string status = "???";

		if (pairs.size())
		{
			vector<string> split = Explode(pairs.back(), ' ');

			if (split.size() == 2)
			{
				hash = split[0];
				status = split[1];
			}
		}


		if (status == "passed")
		{
			File << "<span class=\"label label-success\">Passed</span>" << endl;
		}
		else if (status == "test_failure")
		{
			File << "<span class=\"label label-warning\">Test Failure</span>" << endl;
		}
		else if (status == "build_failure")
		{
			File << "<span class=\"label label-danger\">Build Failure</span>" << endl;
		}
		else
		{
			File << "<span class=\"label label-default\">" << status << "</span>" << endl;
		}

		if (hash != "none")
		{
			File << "</td><td><a href=\"" << assignment << "/" << hash << "/report.html\">" << hash << "</a>" << endl;
		}
		else
		{
			File << "</td><td>" << hash << endl;
		}
		File << "</td></tr>" << endl;
	}

	File << "</tbody></table>" << endl;

	File.close();


	File.open("index.html");

	Cat(TemplateDirectory + "top1.html", File);
	File << "<title>[" << Student << "] CPE 473 Grade Results</title>" << endl;
	Cat(TemplateDirectory + "top2.html", File);
	File << "<h1>[CPE 473] All Program Grade Results</h1>" << endl;
	Cat("table.html", File);
	string const repo = ReadAsString(AllStudentsDirectory + Student + "/" + "link");
	File << "<p>Repo: <a href=\"" << repo << "\">" << repo << "</a></p>" << endl;
	Cat(TemplateDirectory + "bottom.html", File);

	File.close();
}

std::string replace_all(std::string subject, std::string const & search, std::string const & with) {
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos)
	{
		subject.replace(pos, search.length(), with);
		pos += with.length();
	}
	return subject;
}

void IndexBuilder::GenerateCompleteIndex()
{
	File.open("all.html");

	Cat(TemplateDirectory + "top1.html", File);
	File << "<title>CPE 473 Grade Results</title>" << endl;
	Cat(TemplateDirectory + "top2.html", File);
	File << "<h1>[CPE 473] All Student Grade Results</h1>" << endl;

	vector<string> students = ReadAsLines(AllStudentsDirectory + "list");
	for (string const & student : students)
	{
		string table = ReadAsString(student + "/table.html");
		table = replace_all(table, "href=\"", "href=\"" + student + "/");
		File << table;

		string const repo = ReadAsString(AllStudentsDirectory + student + "/" + "link");
		File << "<p>Repo: <a href=\"" << repo << "\">" << repo << "</a></p>" << endl;
		File << "<hr />" << endl;
	}
	Cat(TemplateDirectory + "bottom.html", File);

	File.close();
}