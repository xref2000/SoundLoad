#pragma once

namespace err
{
	inline std::mutex log_mutex;

	template <typename... t> void warn(std::format_string<t...> fmt, t&&... args)
	{
		const std::lock_guard<std::mutex> lock(log_mutex);
		std::cerr << "WARNING: " << std::format(fmt, std::forward<t>(args)...) << '\n';
	}

	template <typename... t> void warn_ex(std::format_string<t...> fmt, t&&... args)
	{
		const std::lock_guard<std::mutex> lock(log_mutex);
		std::cerr << "WARNING: " << std::format(fmt, std::forward<t>(args)...) << "\nError code: 0x" << std::hex << GetLastError() << std::dec << '\n';
	}

	template <typename... t> void log(std::format_string<t...> fmt, t&&... args)
	{
		const std::lock_guard<std::mutex> lock(log_mutex);
		std::cerr << "ERROR: " << std::format(fmt, std::forward<t>(args)...) << '\n';
	}

	template <typename... t> void log(std::wformat_string<t...> fmt, t&&... args)
	{
		const std::lock_guard<std::mutex> lock(log_mutex);
		std::wcerr << L"ERROR: " << std::format(fmt, std::forward<t>(args)...) << L'\n';
	}

	template <typename... t> void log_ex(std::format_string<t...> fmt, t&&... args)
	{
		const std::lock_guard<std::mutex> lock(log_mutex);
		std::cerr << "ERROR: " << std::format(fmt, std::forward<t>(args)...) << "\nError code: 0x" << std::hex << GetLastError() << std::dec << '\n';
	}

	inline void log_net(const cpr::Response& r)
	{
		const std::lock_guard<std::mutex> lock(log_mutex);
		std::cerr << "ERROR: request to \"" << r.url << "\" failed with code " << r.status_code << '\n';
	}
}