#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

#include <iostream>
#include <print>
#include <fstream>
#include <string>
#include <filesystem>
#include <regex>
#include <vector>
#include <deque>

#include "json/json.hpp"

#include <cpr/cpr.h>

#include <taglib/fileref.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/mp4file.h>
#include <taglib/mp4tag.h>
#include <taglib/mp4coverart.h>

extern "C"
{

#include "libavformat/avformat.h"
#include "libavutil/error.h"

}

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