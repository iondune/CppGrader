
#pragma once

#include "Util.hpp"
#include <json.hpp>


enum class ETestType
{
	//! Textual diff
	Text,

	//! Image diff
	Image,

	//! User-provided .pov, not diffed
	Additional
};

struct Test
{
	string Name;
	ETestType Type;
	bool Required = true;
	float Timeout = 20.f;
};

void from_json(nlohmann::json const & j, Test & t);
