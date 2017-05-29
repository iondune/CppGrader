
#pragma once

#include "Util.hpp"
#include "Directories.hpp"


class IndexBuilder
{

public:

	IndexBuilder(string const & student, string const & assignment);

	void GenerateAssignmentIndex();
	void GenerateStudentIndex();

protected:

	string Student;
	string Assignment;

	std::ofstream File;

};
