
#pragma once

#include "Util.hpp"
#include "Directories.hpp"


class HTMLBuilder
{

public:

	HTMLBuilder(string const & student, string const & assignment);

	void Generate();

protected:

	string Student;
	string Assignment;

	string status;

	std::ofstream File;

	void header_info(string const & student, string const & assignment);
	bool build_info();
	void text_tests();

	void cleanup();

	void collapse_button(string const & id);
	void modal_window_start(string const & id, string const & button_label, string const & btn_class);
	void modal_window_end();

};
