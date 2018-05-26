
#include "Test.hpp"

using nlohmann::json;


void from_json(nlohmann::json const & j, Test & t)
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
	else if (type == "additional")
	{
		t.Type = ETestType::Additional;
	}
	else
	{
		throw std::runtime_error("could not parse test type: "s + type);
	}

	if (t.Type == ETestType::Additional)
	{
		t.Required = false;
	}
	else
	{
		t.Required = j.at("required").get<bool>();
	}

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
