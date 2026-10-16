#include "pch.hpp"
#include "logs.hpp"
#include "config.hpp"
#include "utils/utils.hpp"

namespace cfg
{
	//
	//// ARGUMENT PARSING
	//

	static void show_usage(void)
	{
		std::println("[*] ABOUT THIS PROGRAM");
		std::println("All valid arguments are listed below. You MUST surround any arguments with cmd operators (e.g. < | & ?) or spaces in quotes. THIS INCLUDES LINKS WITH '?' IN THEM!");
		std::println("Some features provide short and long versions of the same argument. The short version is in parenthesis at the start of the line, if it exists.");
		std::println("For detailed usage instructions: https://github.com/xref2000/SoundLoad");

		std::println("\n[+] GENERAL CONFIGURATION");
		std::println("-audio     ; [SAVEABLE, DEFAULT] enables the download of audio files");
		std::println("-n-audio   ; [SAVEABLE] disables the download of audio files");
		std::println("-art       ; [SAVEABLE] enables cover art downloads");
		std::println("-n-art     ; [SAVEABLE, DEFAULT] disables cover art downloads");
		std::println("-pvars     ; adds the program to PATH variables");
		std::println("-cid <cid> ; [SAVEABLE] sets the client ID (use only if you know what you're doing)");
		std::println("(-s) -save ; saves applicable arguments to cfg.json");

		std::println("\n[+] FILE INFO");
		std::println("(-adst)  -audio-dst <dir>   ; [SAVEABLE] sets the download directory for audio files");
		std::println("(-idst)  -img-dst <dir>     ; [SAVEABLE] sets the download directory for images");
		std::println("(-aname) -audio-name <name> ; the name to be given to the downloaded audio file");
		std::println("(-iname) -img-name <name>   ; the name to be given to the downloaded cover art");
		std::println("(-isrc)  -img-src <source>  ; the cover art that will be added to the audio (soundcloud track link, image link, or image path)");

		std::println("\n[+] METADATA");
		std::println("(-t)  -title <title       ; title property of audio tag (also used for the file name if one is not specified)");
		std::println("(-c)  -comment <comment>  ; comment property of the audio tag (automatically adds the upload date, upload description, and tags)");
		std::println("(-a)  -artists <artists>  ; contributing artists property of audio tag (defaults to artist metadata or uploader)");
		std::println("(-aa) -a-artist <artist>  ; album artist property of audio tag (defaults to album uploader if in an album)");
		std::println("(-al) -album <album>      ; album property of audio tag (defaults to track name or, if track is in an album, the album name)");
		std::println("(-g)  -genre <genre>      ; genre property of audio tag (defaults to parsed genre from track)");
		std::println("(-n)  -num <track number> ; track number property of audio tag (defaults to track number in album if track is in an album)");
		std::println("(-y)  -year <year>        ; year property of audio tag (defaults to year of upload date)");
	}

	bool parse_arguments(int argc, wchar_t* argv[])
	{
		// Checking arg count

		if (argc < 2)
		{
			err::log("insufficient arguments, use '-?' for program usage");
			return false;
		}

		// Checking if first argument (link required for any downloads)
		
		if (_wcsnicmp(argv[1], L"https://soundcloud.com/", 23) != 0)
		{
			cfg::f.no_link_provided = true;

			if (!wcscmp(argv[1], L"-?"))
			{
				show_usage();
				return false;
			}
		}
		
		// Parsing arguments (starts at index 2 if first arg is a link)

		for (int i = 1 + ((cfg::f.no_link_provided) == false); i < argc; ++i)
		{
			const wchar_t* arg = argv[i];
			auto log_bad_arg = [&arg](void) { err::log(L"invalid argument \"{}\"", arg); };

			if (arg[0] != '-' || !arg[1])
			{
				log_bad_arg();
				return false;
			}

			const size_t arg_hash = HASH_RT(&arg[1]);
			bool handled = true;

			switch (arg_hash)
			{
			case HASH("s"):       { [[fallthrough]];                        }
			case HASH("save"):    { cfg::f.save_cfg          = true; break; }
			case HASH("art"):     { cfg::f.seperate_art      = true; break; }
			case HASH("n-art"):   { cfg::f.no_seperate_art   = true; break; }
			case HASH("audio"):   { cfg::f.download_audio    = true; break; }
			case HASH("n-audio"): { cfg::f.no_audio_download = true; break; }
			case HASH("pvars"):   { cfg::f.add_to_path       = true; break; }

			default: handled = false;
			}

			if (!handled)
			{
				++i;

				if (i >= argc)
				{
					err::log(L"no value provided for argument \"{}\"", arg);
					return false; 
				}

				const wchar_t* next_arg = argv[i];
				if (!next_arg)
				{
					err::log("argv/argc mismatch occured, retry or recompile the program");
					return false;
				}

				switch (arg_hash)
				{
					// Config data arguments

				case HASH("iname"): [[fallthrough]];
				case HASH("img-name"):   { cfg::g_data.image_name = next_arg; break; }

				case HASH("aname"): [[fallthrough]];
				case HASH("audio-name"): { cfg::g_data.audio_name = next_arg; break; }

				case HASH("idst"):  [[fallthrough]];
				case HASH("img-dst"):    { cfg::image_dir         = next_arg; break; }

				case HASH("adst"):  [[fallthrough]];
				case HASH("audio-dst"):  { cfg::audio_dir         = next_arg; break; }

				case HASH("isrc"):  [[fallthrough]];
				case HASH("img-src"):    { cfg::image_src         = next_arg; break; }

					// Audio tag arguments

				case HASH("t"):  [[fallthrough]];
				case HASH("title"):    { cfg::g_data.title           = next_arg; break; }

				case HASH("c"):  [[fallthrough]];
				case HASH("comment"):  { cfg::g_data.comments        = next_arg; break; }

				case HASH("a"):  [[fallthrough]];
				case HASH("artists"):  { cfg::g_data.contrib_artists = next_arg; break; }

				case HASH("aa"): [[fallthrough]];
				case HASH("a-artist"): { cfg::g_data.album_artists   = next_arg; break; }

				case HASH("al"): [[fallthrough]];
				case HASH("album"):    { cfg::g_data.album           = next_arg; break; }

				case HASH("g"):  [[fallthrough]];
				case HASH("genre"):    { cfg::g_data.genre           = next_arg; break; }

				case HASH("n"):  [[fallthrough]];
				case HASH("num"):      { cfg::g_data.number          = next_arg; break; }

				case HASH("y"):  [[fallthrough]];
				case HASH("year"):     { cfg::g_data.year            = next_arg; break; }

					// Extra

				case HASH("cid"):
				{
					cfg::client_id.resize(lstrlenW(next_arg));
					WideCharToMultiByte(CP_UTF8, 0, next_arg, -1, cfg::client_id.data(), static_cast<int>(cfg::client_id.size()), nullptr, nullptr);
					break;
				}

					// Invalid arguments

				default:
				{
					log_bad_arg();
					return false;
				}
				}
			}
		}

		// Getting cover art source type

		if (!cfg::image_src.empty())
		{
			if (std::filesystem::exists(cfg::image_src))
			{
				cfg::f.cover_src_path = true;
			}
			else if (cfg::image_src.starts_with(L"https://soundcloud.com/"))
			{
				cfg::f.cover_src_sc_link = true;
			}
		}

		// Getting process directory

		wchar_t program_path[MAX_PATH];

		if (!GetModuleFileNameW(nullptr, program_path, MAX_PATH))
		{
			err::log_ex("failed to get sl.exe path");
			return false;
		}

		cfg::path_cache = std::filesystem::path(program_path).parent_path().wstring();
		return true;
	}

	//
	//// CONFIG HANDLING
	//

	void read_config(const cfg_format& data)
	{
		if (!data.cid.empty() && cfg::client_id.empty())
		{
			cfg::client_id = data.cid;
		}

		if (!data.art_out_dir.empty() && cfg::image_dir.empty())
		{
			cfg::image_dir = data.art_out_dir;
		}

		if (!data.track_out_dir.empty() && cfg::audio_dir.empty())
		{
			cfg::audio_dir = data.track_out_dir;
		}

		if (!cfg::audio_flags_set() && !data.get_track_audio)
		{
			cfg::f.no_audio_download = true;
		}

		if (!cfg::art_flags_set() && data.get_track_art)
		{
			cfg::f.seperate_art = true;
		}
	}

	void save_config(cfg_format& data)
	{
		if (!cfg::client_id.empty())
		{
			data.cid = cfg::client_id;
		}

		if (!cfg::image_dir.empty())
		{
			data.art_out_dir = cfg::image_dir;
		}

		if (!cfg::audio_dir.empty())
		{
			data.track_out_dir = cfg::audio_dir;
		}

		if (cfg::audio_flags_set())
		{
			data.get_track_audio = cfg::f.download_audio;
		}

		if (cfg::art_flags_set())
		{
			data.get_track_art = cfg::f.seperate_art;
		}
	}

	//
	//// PATH variable
	//

	void add_to_path(void)
	{
		// Getting 'HKCU/Environment/Path' value

		HKEY hkey = nullptr;
		if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Environment", 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkey) != ERROR_SUCCESS)
		{
			err::warn_ex("failed to open HKCU/Environment");
			return;
		}

		DWORD cbSize = 0;
		if (RegGetValueW(hkey, nullptr, L"Path", RRF_NOEXPAND | RRF_RT_REG_EXPAND_SZ, nullptr, nullptr, &cbSize) != ERROR_SUCCESS)
		{
			err::warn_ex("failed to query size of HKCU/Environment/Path");
			RegCloseKey(hkey);
			return;
		}

		std::wstring value(cbSize / sizeof(WCHAR), 0);
		if (RegGetValueW(hkey, nullptr, L"Path", RRF_NOEXPAND | RRF_RT_REG_EXPAND_SZ, nullptr, value.data(), &cbSize) != ERROR_SUCCESS)
		{
			err::warn_ex("failed to query value of HKCU/Environment/Path");
			RegCloseKey(hkey);
			return;
		}

		// Formatting value

		value.erase(value.find_first_of(L'\0')); // there are multiple null terminators sometimes
		if (value.back() != L';') value.push_back(L';');

		std::transform(value.begin(), value.end(), value.begin(), ::tolower);
		std::transform(cfg::path_cache.begin(), cfg::path_cache.end(), cfg::path_cache.begin(), ::tolower);

		// Setting value

		if (value.find(cfg::path_cache) == std::wstring::npos && (cfg::f.add_to_path || MessageBoxW(nullptr, L"Add sl.exe directory to PATH variables? If done, cmd will recognize sl.exe no matter the working directory.", L"SoundLoad", MB_YESNO | MB_ICONQUESTION) == IDYES))
		{
			value += cfg::path_cache + L';';

			if (RegSetValueExW(hkey, L"Path", 0, REG_EXPAND_SZ, reinterpret_cast<BYTE*>(value.data()), static_cast<DWORD>(value.size() * sizeof(WCHAR))) == ERROR_SUCCESS)
			{
				std::cout << "[!] Added sl.exe directory to PATH variables\n";
			}
			else 
			{
				err::warn("failed to set HKCU/Environment/Path value");
			}
		}

		RegCloseKey(hkey);
	}
}