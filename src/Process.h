
#pragma once

#include "Util.h"
#include <iostream>
#include <stdexcept>
#include <subprocess.hpp>

namespace sp = subprocess;


template <typename... Args>
void required_command(std::initializer_list<string> const & cmd, Args&&... args)
{
	auto p = sp::Popen(cmd, sp::output{sp::PIPE}, sp::error{sp::STDOUT}, std::forward<Args>(args)...);
	auto res = p.communicate();
	auto retcode = p.poll();
	if (retcode > 0)
	{
		throw sp::CalledProcessError("Command failed: Non zero retcode");
	}
	std::cout << res.first.buf.data();
}

template <typename... Args>
string required_command_output(std::initializer_list<string> const & cmd, Args&&... args)
{
	auto p = sp::Popen(cmd, sp::output{sp::PIPE},  sp::error{sp::STDOUT}, std::forward<Args>(args)...);
	auto res = p.communicate();
	auto retcode = p.poll();
	if (retcode > 0)
	{
		throw sp::CalledProcessError("Command failed: Non zero retcode");
	}
	return res.first.buf.data();
}

template <typename... Args>
bool try_command_redirect(std::initializer_list<string> const & cmd, string const & filename, Args&&... args)
{
	FILE * file = fopen(filename.c_str(), "w");
	if (! file)
	{
		throw std::runtime_error(std::string("Can't open file: ") + filename);
	}
	auto p = sp::Popen(cmd, sp::output{file}, sp::error{sp::STDOUT}, std::forward<Args>(args)...);
	auto res = p.communicate();
	auto retcode = p.poll();
	return retcode == 0;
}
