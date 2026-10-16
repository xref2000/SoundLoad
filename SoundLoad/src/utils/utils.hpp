#pragma once

//
//// UTILS
//

namespace utils
{
	//
	//// FNV HASHING (https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function)
	//

	namespace fnv
	{
		consteval unsigned long hash(const wchar_t* input, const unsigned long val = 0x811C9DC5u) noexcept
		{
			wchar_t ch = input[0];
			if (ch < 91 && ch > 64) 
				ch += 32;

			return !ch ? val : hash(input + 1, (val ^ ch) * 0x01000193u);
		}

		inline constexpr unsigned long hash_rt(const wchar_t* input, const unsigned long val = 0x811C9DC5u) noexcept
		{
			wchar_t ch = input[0];
			if (ch < 91 && ch > 64) 
				ch += 32;

			return !ch ? val : hash_rt(input + 1, (val ^ ch) * 0x01000193u);
		}

		consteval unsigned long hash(const char* input, const unsigned long val = 0x811C9DC5u) noexcept
		{
			char ch = input[0];
			if (ch < 91 && ch > 64)
				ch += 32;

			return !ch ? val : hash(input + 1, (val ^ ch) * 0x01000193u);
		}

		inline constexpr unsigned long hash_rt(const char* input, const unsigned long val = 0x811C9DC5u) noexcept
		{
			char ch = input[0];
			if (ch < 91 && ch > 64)
				ch += 32;

			return !ch ? val : hash_rt(input + 1, (val ^ ch) * 0x01000193u);
		}
	}

	//
	//// STRING HELPERS
	//

	namespace str
	{
		inline void wide_to_mb(const std::wstring& src, std::string& dst)
		{
			if (!src.empty())
			{
				dst.resize(src.size());
				WideCharToMultiByte(CP_UTF8, 0, src.c_str(), -1, dst.data(), static_cast<int>(dst.size()), nullptr, nullptr);
			}
		}

		inline void mb_to_wide(const std::string& src, std::wstring& dst)
		{
			if (!src.empty())
			{
				dst.resize(src.size());
				MultiByteToWideChar(CP_UTF8, 0, src.c_str(), -1, dst.data(), static_cast<int>(dst.size()));
			}
		}
	}
}

//
//// MACROS
//

#define HASH(input)    utils::fnv::hash(input)
#define HASH_RT(input) utils::fnv::hash_rt(input)