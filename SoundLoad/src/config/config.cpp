#include "pch.hpp"
#include "config.hpp"

namespace cfg
{
	//
	//// ARGUMENT PARSING
	//

	static void show_usage(void)
	{
		std::cout << "[*] ABOUT THIS PROGRAM\n";
		std::cout << "All valid arguments are listed below. You MUST surround any arguments with cmd operators (e.g. < | & ?) or spaces in quotes. THIS INCLUDES LINKS WITH '?' IN THEM!\n";
		std::cout << "Some features provide short and long versions of the same argument. The short version is in parenthesis at the start of the line, if it exists.\n";
		std::cout << "For detailed usage instructions: https://github.com/xref2000/SoundLoad\n";

		std::cout << "\n[+] GENERAL CONFIGURATION\n";
		std::cout << "-audio     ; [SAVEABLE, DEFAULT] enables the download of audio files\n";
		std::cout << "-n-audio   ; [SAVEABLE] disables the download of audio files\n";
		std::cout << "-art       ; [SAVEABLE] enables cover art downloads\n";
		std::cout << "-n-art     ; [SAVEABLE, DEFAULT] disables cover art downloads\n";
		std::cout << "-aac       ; [SAVEABLE] enables AAC audio download (apple music and spotify wont play these files)\n";
		std::cout << "-n-aac     ; [SAVEABLE, DEFAULT] disables AAC audio download (highest bitrate MPEG used)\n";
		std::cout << "-pvars     ; adds the program to PATH variables\n";
		std::cout << "-cid <cid> ; [SAVEABLE] sets the client ID (use only if you know what you're doing)\n";
		std::cout << "(-s) -save ; saves applicable arguments to cfg.json\n";

		std::cout << "\n[+] FILE INFO\n";
		std::cout << "(-adst)  -audio-dst <dir>   ; [SAVEABLE] sets the download directory for audio files\n";
		std::cout << "(-idst)  -img-dst <dir>     ; [SAVEABLE] sets the download directory for images\n";
		std::cout << "(-aname) -audio-name <name> ; the name to be given to the downloaded audio file\n";
		std::cout << "(-iname) -img-name <name>   ; the name to be given to the downloaded cover art\n";
		std::cout << "(-isrc)  -img-src <source>  ; the cover art that will be added to the audio (soundcloud track link, image link, or image path)\n";
		
		std::cout << "\n[+] METADATA\n";
		std::cout << "(-t)  -title <title       ; title property of audio tag (also used for the file name if one is not specified)\n";
		std::cout << "(-c)  -comment <comment>  ; comment property of the audio tag (automatically adds the upload date, upload description, and tags)\n";
		std::cout << "(-a)  -artists <artists>  ; contributing artists property of audio tag (defaults to artist metadata or uploader)\n";
		std::cout << "(-aa) -a-artist <artist>  ; album artist property of audio tag (defaults to album uploader if in an album)\n";
		std::cout << "(-al) -album <album>      ; album property of audio tag (defaults to track name or, if track is in an album, the album name)\n";
		std::cout << "(-g)  -genre <genre>      ; genre property of audio tag (defaults to parsed genre from track)\n";
		std::cout << "(-n)  -num <track number> ; track number property of audio tag (defaults to track number in album if track is in an album)\n";
		std::cout << "(-y)  -year <year>        ; year property of audio tag (defaults to year of upload date)\n";
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
			PCWSTR arg = argv[i];
			auto log_bad_arg = [&arg](void) 
				{ 
					err::log(L"invalid argument \"{}\"", arg); 
				};

			if (arg[0] != '-' || !arg[1])
			{
				log_bad_arg();
				return false;
			}

			const size_t arg_hash = hash_rt(&arg[1]);
			bool handled = true;

			switch (arg_hash)
			{
			case hash(L"s"):       { [[fallthrough]];                             }
			case hash(L"save"):    { cfg::f.save_config            = true; break; }
			case hash(L"art"):     { cfg::f.download_art_seperate  = true; break; }
			case hash(L"n-art"):   { cfg::f.disable_art_download   = true; break; }
			case hash(L"audio"):   { cfg::f.download_audio         = true; break; }
			case hash(L"n-audio"): { cfg::f.disable_audio_download = true; break; }
			case hash(L"pvars"):   { cfg::f.add_to_path            = true; break; }
			case hash(L"aac"):     { cfg::f.get_aac_transcoding    = true; break; }
			case hash(L"n-aac"):   { cfg::f.no_aac_transcodings    = true; break; }

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

				PCWSTR next_arg = argv[i];
				if (!next_arg)
				{
					err::log("argv/argc mismatch occured, retry or recompile the program");
					return false;
				}

				bool stoul_failed = false;
				auto handle_stoul = [&stoul_failed, &next_arg](UINT& buffer)
					{
						try { buffer = std::stoul(next_arg); }
						catch (...) { stoul_failed = true; }
					};

				switch (arg_hash)
				{
					// Config data arguments

				case hash(L"iname"): [[fallthrough]];
				case hash(L"img-name"):   { cfg::g_data.image_file_name = next_arg; break; }

				case hash(L"aname"): [[fallthrough]];
				case hash(L"audio-name"): { cfg::g_data.audio_file_name = next_arg; break; }

				case hash(L"idst"):  [[fallthrough]];
				case hash(L"img-dst"):    { cfg::image_out_dir          = next_arg; break; }

				case hash(L"adst"):  [[fallthrough]];
				case hash(L"audio-dst"):  { cfg::audio_out_dir          = next_arg; break; }

				case hash(L"isrc"):  [[fallthrough]];
				case hash(L"img-src"):    { cfg::image_src              = next_arg; break; }

					// Audio tag arguments

				case hash(L"t"):  [[fallthrough]];
				case hash(L"title"):    { cfg::g_data.title           = next_arg; break; }

				case hash(L"c"):  [[fallthrough]];
				case hash(L"comment"):  { cfg::g_data.comments        = next_arg; break; }

				case hash(L"a"):  [[fallthrough]];
				case hash(L"artists"):  { cfg::g_data.contrib_artists = next_arg; break; }

				case hash(L"aa"): [[fallthrough]];
				case hash(L"a-artist"): { cfg::g_data.album_artists   = next_arg; break; }

				case hash(L"al"): [[fallthrough]];
				case hash(L"album"):    { cfg::g_data.album           = next_arg; break; }

				case hash(L"g"):  [[fallthrough]];
				case hash(L"genre"):    { cfg::g_data.genre           = next_arg; break; }

				case hash(L"n"):  [[fallthrough]];
				case hash(L"num"):      { handle_stoul(cfg::g_data.number); break; }

				case hash(L"y"):  [[fallthrough]];
				case hash(L"year"):     { handle_stoul(cfg::g_data.year);   break; }

					// Extra

				case hash(L"cid"):
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

				if (stoul_failed)
				{
					log_bad_arg();
					return false;
				}
			}
		}

		// Getting cover art source type

		if (!cfg::image_src.empty())
		{
			if (std::filesystem::exists(cfg::image_src))
			{
				cfg::f.cover_src_is_path = true;
			}
			else if (cfg::image_src.starts_with(L"https://soundcloud.com/"))
			{
				cfg::f.cover_src_is_sc_link = true;
			}
		}

		// Getting process directory

		WCHAR program_path[MAX_PATH];

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

		if (!data.art_out_dir.empty() && cfg::image_out_dir.empty())
		{
			cfg::image_out_dir = data.art_out_dir;
		}

		if (!data.track_out_dir.empty() && cfg::audio_out_dir.empty())
		{
			cfg::audio_out_dir = data.track_out_dir;
		}

		if (!cfg::audio_flags_set() && !data.get_track_audio)
		{
			cfg::f.disable_audio_download = true;
		}

		if (!cfg::art_flags_set() && data.get_track_art)
		{
			cfg::f.download_art_seperate = true;
		}

		if (!cfg::aac_flags_set() && data.get_aac_transcoding)
		{
			cfg::f.get_aac_transcoding = true;
		}
	}

	void save_config(cfg_format& data)
	{
		if (!cfg::client_id.empty())
		{
			data.cid = cfg::client_id;
		}

		if (!cfg::image_out_dir.empty())
		{
			data.art_out_dir = cfg::image_out_dir;
		}

		if (!cfg::audio_out_dir.empty())
		{
			data.track_out_dir = cfg::audio_out_dir;
		}

		if (cfg::audio_flags_set())
		{
			data.get_track_audio = cfg::f.download_audio;
		}

		if (cfg::art_flags_set())
		{
			data.get_track_art = cfg::f.download_art_seperate;
		}

		if (cfg::aac_flags_set())
		{
			data.get_aac_transcoding = cfg::f.get_aac_transcoding;
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

		if (value.find(cfg::path_cache) == std::wstring::npos
		&& (cfg::f.add_to_path || MessageBoxW(nullptr, L"Add sl.exe directory to PATH variables? If done, cmd will recognize sl.exe no matter the working directory.", L"SoundLoad", MB_YESNO | MB_ICONQUESTION) == IDYES))
		{
			value += cfg::path_cache + L';';

			if (RegSetValueExW(hkey, L"Path", 0, REG_EXPAND_SZ, reinterpret_cast<BYTE*>(value.data()), static_cast<DWORD>(value.size() * sizeof(WCHAR))) == ERROR_SUCCESS)
			{
				std::cout << "[!] Added sl.exe directory to PATH variables\n";
			}
			else err::warn("failed to set HKCU/Environment/Path value");
		}

		RegCloseKey(hkey);
	}
}