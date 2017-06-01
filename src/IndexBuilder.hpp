
#pragma once

#include "Util.hpp"
#include "Directories.hpp"


struct CommitInfo
{
	string Hash;
	string Status;
	time_t Date;
	string DateString;

	bool operator < (CommitInfo const & other) const
	{
		return Date < other.Date;
	}
};

class IndexBuilder
{

public:

	IndexBuilder(string const & student, string const & assignment, string const & repoDirectory);

	void GenerateAssignmentIndex();
	void GenerateStudentIndex();
	void GenerateCompleteIndex();

protected:

	string Student;
	string Assignment;
	string RepoDirectory;

	vector<CommitInfo> GetCommits();

};
