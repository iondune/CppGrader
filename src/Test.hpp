
#pragma once

#include "Util.hpp"
#include <json.hpp>


enum class ETestType
{
	Text,
	Image
};

struct Test
{
	string Name;
	ETestType Type;
	bool Required = true;
	float Timeout = 20.f;
};

void from_json(nlohmann::json const & j, Test & t);
