#pragma once

namespace err
{
	inline std::mutex log_mutex;

	template <typename... t> 
	void warn(std::format_string<t...> fmt, t&&... args)
	{
		const std::lock_guard<std::mutex> lock(log_mutex);
		std::println(stderr, "WARNING: {}", std::format(fmt, std::forward<t>(args)...));
	}

	template <typename... t> 
	void warn_ex(std::format_string<t...> fmt, t&&... args)
	{
		const std::lock_guard<std::mutex> lock(log_mutex);
		std::println(stderr, "WARNING: {}\nError code: 0x{:x}", std::format(fmt, std::forward<t>(args)...), GetLastError());
	}

	template <typename... t> 
	void log(std::format_string<t...> fmt, t&&... args)
	{
		const std::lock_guard<std::mutex> lock(log_mutex);
		std::println(stderr, "ERROR: {}", std::format(fmt, std::forward<t>(args)...));
	}

	template <typename... t> 
	void log(std::wformat_string<t...> fmt, t&&... args)
	{
		const std::lock_guard<std::mutex> lock(log_mutex);
		std::wcerr << L"ERROR: " << std::format(fmt, std::forward<t>(args)...) << L'\n';
	}

	template <typename... t> 
	void log_ex(std::format_string<t...> fmt, t&&... args)
	{
		const std::lock_guard<std::mutex> lock(log_mutex);
		std::println(stderr, "ERROR: {}\nError code: 0x{:x}", std::format(fmt, std::forward<t>(args)...), GetLastError());
	}

	inline void log_net(const cpr::Response& r)
	{
		const std::lock_guard<std::mutex> lock(log_mutex);
		std::println(stderr, "ERROR: request to \"{}\" failed with code {}", r.url.str(), r.status_code);
	}
}