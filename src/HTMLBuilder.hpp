
#pragma once

#include "Util.hpp"
#include "Directories.hpp"


class HTMLBuilder
{

public:

	HTMLBuilder(string const & student, string const & assignment, string const & hash);

	void Generate();

protected:

	string Student;
	string Assignment;
	string Status;
	string Hash;

	std::ofstream File;

	void HeaderInfo();
	bool BuildInfo();
	void TextTests();
	void ImageTests();
	void AdditionalImages();

	void Cleanup();

	void CollapseButton(string const & id);
	void ModalWindowStart(string const & id, string const & button_label, string const & btn_class);
	void ModalWindowEnd();

};
