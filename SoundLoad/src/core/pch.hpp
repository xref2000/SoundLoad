#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <regex>
#include <vector>

#include "json/json.hpp"

#include "cpr/cpr.h"

#include "taglib/fileref.h"
#include "taglib/mpeg/mpegfile.h"
#include "taglib/mpeg/id3v2/id3v2tag.h"
#include "taglib/mpeg/id3v2/frames/attachedpictureframe.h"
#include "taglib/mp4/mp4file.h"
#include "taglib/mp4/mp4tag.h"
#include "taglib/mp4/mp4coverart.h"

/* nlohmann::json modifications
* 
* [*] nlohmann::json::value
* 
* > NEW OVERLOAD
*   - defaults ReturnType to std::string
*   - removes default_value argument, will instead return ReturnType{} by default
*   - catches nlohmann::json::type_error exception 302 and returns ReturnType{} (properly handles cases of existing null values)
* 
* > MODIFIED OVERLOAD
*   - catches nlohmann::json::type_error exception 302 and returns default_value (properly handles cases of existing null values)
*/

using Json = nlohmann::json;

// COMPILE/RUNTIME FNV1A HASHING (case-insensitive)

consteval uint32_t hash(const wchar_t* input, const uint32_t val = 0x811C9DC5u) noexcept
{
	wchar_t ch = input[0];
	if (ch < 91 && ch > 64) ch += 32;
	return !ch ? val : hash(input + 1, (val ^ ch) * 0x01000193u);
}

inline constexpr uint32_t hash_rt(const wchar_t* input, const uint32_t val = 0x811C9DC5u) noexcept
{
	wchar_t ch = input[0];
	if (ch < 91 && ch > 64) ch += 32;
	return !ch ? val : hash_rt(input + 1, (val ^ ch) * 0x01000193u);
}