
#include "IndexBuilder.hpp"
#include "FileSystem.hpp"
#include "Process.hpp"


IndexBuilder::IndexBuilder(string const & student, string const & assignment, string const & repoDirectory)
{
	this->Student = student;
	this->Assignment = assignment;
	this->RepoDirectory = repoDirectory;
}

void IndexBuilder::GenerateAssignmentIndex()
{
	std::ofstream File;
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
	File << "<th>Time</th>" << endl;
	File << "</tr>" << endl;
	File << "</thead>" << endl;
	File << "<tbody>" << endl;

	vector<CommitInfo> const Commits = GetCommits();

	for (CommitInfo const & Commit : Commits)
	{
		File << "<tr>" << endl;
		File << "<td><a href=\"" << Commit.Hash << "/report.html\">" << Commit.Hash << "</a></td>" << endl;

		File << "<td>";
		if (Commit.Status == "passed")
		{
			File << "<span class=\"label label-success\">Passed</span>";
		}
		else if (Commit.Status == "test_failure")
		{
			File << "<span class=\"label label-warning\">Test Failure</span>";
		}
		else if (Commit.Status == "build_failure")
		{
			File << "<span class=\"label label-danger\">Build Failure</span>";
		}
		else
		{
			File << "<span class=\"label label-default\">" << Commit.Status << "</span>";
		}
		File << "</td>" << endl;

		File << "<td>" << Commit.DateString << "</td>" << endl;

		File << "</tr>" << endl;
	}

	File << "</tbody></table>" << endl;

	Cat(TemplateDirectory + "bottom.html", File);

	File.close();
}

void IndexBuilder::GenerateStudentIndex()
{
	std::ofstream File;
	File.open("table.html");

	File << "<p>Student: " << Student << "</p>" << endl;

	File << "<table class=\"table table-striped table-bordered\" style=\"width: auto;\">" << endl;
	File << "<thead>" << endl;
	File << "<tr>" << endl;
	File << "<th>Assignment</th>" << endl;
	File << "<th>Status</th>" << endl;
	File << "<th>Latest Graded Commit</th>" << endl;
	File << "<th>Latest Commit Date</th>" << endl;
	File << "</tr>" << endl;
	File << "</thead>" << endl;
	File << "<tbody>" << endl;

	vector<string> assignments = ReadAsLines(ExecDirectory + "assignments");

	for (string const & assignment : assignments)
	{
		File << "<tr>" << endl;
		File << "<td><a href=\"" << assignment << "/\">" << assignment << "</a></td>" << endl;

		fs::current_path(assignment);
		vector<CommitInfo> const Commits = GetCommits();
		fs::current_path("..");

		string hash = "none";
		string status = "???";
		string date = "???";

		if (Commits.size())
		{
			CommitInfo const Commit = Commits.front();

			hash = Commit.Hash;
			status = Commit.Status;
			date = Commit.DateString;
		}

		File << "<td>";
		if (status == "passed")
		{
			File << "<span class=\"label label-success\">Passed</span>";
		}
		else if (status == "test_failure")
		{
			File << "<span class=\"label label-warning\">Test Failure</span>";
		}
		else if (status == "build_failure")
		{
			File << "<span class=\"label label-danger\">Build Failure</span>";
		}
		else
		{
			File << "<span class=\"label label-default\">" << status << "</span>";
		}
		File << "</td>" << endl;

		if (hash != "none")
		{
			File << "<td><a href=\"" << assignment << "/" << hash << "/report.html\">" << hash << "</a></td>" << endl;
		}
		else
		{
			File << "<td>" << hash << "</td>" << endl;
		}
		File << "<td>" << date << "</td>" << endl;

		File << "</tr>" << endl;
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


void IndexBuilder::GenerateCompleteIndex()
{
	std::ofstream File;
	File.open("all.html");

	Cat(TemplateDirectory + "top1.html", File);
	File << "<title>CPE 473 Grade Results</title>" << endl;
	Cat(TemplateDirectory + "top2.html", File);
	File << "<h1>[CPE 473] All Student Grade Results</h1>" << endl;

	vector<string> students = ReadAsLines(AllStudentsDirectory + "list");
	for (string const & student : students)
	{
		string table = ReadAsString(student + "/table.html");
		table = ReplaceAll(table, "href=\"", "href=\"" + student + "/");
		File << table;

		string const repo = ReadAsString(AllStudentsDirectory + student + "/" + "link");
		File << "<p>Repo: <a href=\"" << repo << "\">" << repo << "</a></p>" << endl;
		File << "<hr />" << endl;
	}
	Cat(TemplateDirectory + "bottom.html", File);

	File.close();
}

vector<CommitInfo> IndexBuilder::GetCommits()
{
	vector<CommitInfo> Commits;

	vector<string> Lines = ReadAsLines("list");

	for (string const & Line : Lines)
	{
		vector<string> Fields = Explode(Line, ' ');

		if (Fields.size() >= 2)
		{
			string const Hash = Fields[0];
			string const Status = Fields[1];

			string const CommitDate = command_output({"git", "log", "-1", "--pretty=format:%ct", Hash, "--"}, sp::environment(std::map<string, string>({{"GIT_DIR", RepoDirectory + ".git/"}})));
			time_t const Date = std::stoi(CommitDate);
			string const DateString = TrimWhitespace(std::ctime(&Date));

			CommitInfo Info;
			Info.Hash = Hash;
			Info.Status = Status;
			Info.Date = Date;
			Info.DateString = DateString;

			auto it = std::find_if(Commits.begin(), Commits.end(), [Info](CommitInfo const & other) { return Info.Hash == other.Hash; });

			if (it != Commits.end())
			{
				// cout << "Found duplicate record for hash " << Hash << ", overwriting." << endl;
				*it = Info;
			}
			else
			{
				Commits.push_back(Info);
			}
		}
	}

	std::sort(Commits.begin(), Commits.end());
	std::reverse(Commits.begin(), Commits.end());

	// for (CommitInfo const & Info : Commits)
	// {
	// 	cout << "Commit Record: " << Info.Hash << ", " << Info.Status << ", " << Info.DateString << endl;
	// }

	return Commits;
}
