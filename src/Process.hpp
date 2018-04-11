
#pragma once

#include "Util.hpp"
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <subprocess.hpp>

namespace sp = subprocess;


namespace
{

template <typename... Args>
string command_output(std::initializer_list<string> const & cmd, Args&&... args)
{
	auto p = sp::Popen(cmd, sp::output{sp::PIPE},  sp::error{sp::STDOUT}, std::forward<Args>(args)...);
	auto res = p.communicate();
	auto retcode = p.retcode();
	return res.first.buf.data();
}

template <typename... Args>
void required_command(std::initializer_list<string> const & cmd, std::ostream & stream = std::cout, Args&&... args)
{
	auto p = sp::Popen(cmd, sp::output{sp::PIPE}, sp::error{sp::STDOUT}, std::forward<Args>(args)...);
	auto res = p.communicate();
	auto retcode = p.retcode();
	if (retcode != 0)
	{
		string command;
		for (auto c : cmd)
			command += " "s + c;
		throw sp::CalledProcessError("Required command failed (non-zero retcode):" + command);
	}
	stream << res.first.buf.data();
}

template <typename... Args>
bool try_command(std::initializer_list<string> const & cmd, std::ostream & stream = std::cout, Args&&... args)
{
	auto p = sp::Popen(cmd, sp::output{sp::PIPE}, sp::error{sp::STDOUT}, std::forward<Args>(args)...);
	auto res = p.communicate();
	auto retcode = p.retcode();
	stream << res.first.buf.data();
	return retcode == 0;
}

template <typename... Args>
string required_command_output(std::initializer_list<string> const & cmd, Args&&... args)
{
	auto p = sp::Popen(cmd, sp::output{sp::PIPE},  sp::error{sp::STDOUT}, std::forward<Args>(args)...);
	auto res = p.communicate();
	auto retcode = p.retcode();
	if (retcode != 0)
	{
		string command;
		for (auto c : cmd)
			command += " "s + c;
		throw sp::CalledProcessError("Required command failed (non-zero retcode):" + command);
	}
	return res.first.buf.data();
}

template <typename... Args>
bool try_command_redirect(std::vector<string> const & cmd, string const & filename, Args&&... args)
{
	FILE * file = fopen(filename.c_str(), "w");
	if (! file)
	{
		throw std::runtime_error(std::string("Can't open file: ") + filename);
	}
	auto p = sp::Popen(cmd, sp::output{file}, sp::error{sp::STDOUT}, std::forward<Args>(args)...);
	auto res = p.communicate();
	int retcode = p.retcode();
	return retcode == 0;
}

enum class ECommandStatus
{
	Success,
	Failure,
	Timeout
};

template <typename... Args>
ECommandStatus try_command_redirect_timeout(std::vector<string> const & cmd, string const & filename, float const max_duration, float & out_duration, Args&&... args)
{
	FILE * file = fopen(filename.c_str(), "w");
	if (! file)
	{
		throw std::runtime_error(std::string("Can't open file: ") + filename);
	}
	auto p = sp::Popen(cmd, sp::output{file}, sp::error{sp::STDOUT}, std::forward<Args>(args)...);

	auto start = std::chrono::steady_clock::now();
	int const timeout = (int) (max_duration * 1000.f);

	int retcode = -1;
	while (1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		retcode = p.poll();
		if (retcode != -1)
		{
			break;
		}

		int const duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
		out_duration = duration / 1000.f;

		if (duration > timeout)
		{
			p.kill();
			return ECommandStatus::Timeout;
		}
	}

	auto res = p.communicate();

	if (retcode)
	{
		return ECommandStatus::Failure;
	}
	else
	{
		return ECommandStatus::Success;
	}
}

}
