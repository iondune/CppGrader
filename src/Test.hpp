
#include "Util.hpp"
#include <json.hpp>

using nlohmann::json;


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

void from_json(json const & j, Test & t)
{
	t.Name = j.at("name").get<string>();
	string type = j.at("type").get<string>();

	if (type == "text")
	{
		t.Type = ETestType::Text;
	}
	else if (type == "image")
	{
		t.Type = ETestType::Image;
	}
	else
	{
		throw std::runtime_error("could not parse test type: "s + type);
	}

	t.Required = j.at("required").get<bool>();

	try
	{
		t.Timeout = j.at("timeout").get<float>();
	}
	catch (std::out_of_range const & e)
	{
		t.Timeout = 20.f;
		if (t.Type == ETestType::Image)
		{
			t.Timeout = 60.f;
		}
	}
}
