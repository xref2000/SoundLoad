#include "pch.hpp"
#include "config.hpp"

namespace cfg
{
	//
	//// ARGUMENT PARSING
	//

	static __forceinline void show_usage()
	{
		std::cout << "[*] ABOUT THIS PROGRAM\n";
		std::cout << "All valid arguments are listed below. You MUST surround any arguments with spaces in quotes.\n";
		std::cout << "Source code: https://github.com/xref2000/SoundLoad\n";

		std::cout << "\n[+] CONFIGURATION\n";
		std::cout << "-cid <client_id>       ; [SAVEABLE] sets the client ID\n";
		std::cout << "-audio-dst <directory> ; [SAVEABLE] sets the download directory for audio files\n";
		std::cout << "-img-dst <directory>   ; [SAVEABLE] sets the download directory for images\n";
		std::cout << "-audio                 ; [SAVEABLE, DEFAULT] enables the download of audio files\n";
		std::cout << "-n-audio               ; [SAVEABLE] disables the download of audio files\n";
		std::cout << "-art                   ; [SAVEABLE] enables cover art downloads\n";
		std::cout << "-n-art                 ; [SAVEABLE, DEFAULT] disables cover art downloads\n";
		std::cout << "-aac                   ; [SAVEABLE] enables AAC audio download (apple music and spotify wont play these files)\n";
		std::cout << "-n-aac                 ; [SAVEABLE, DEFAULT] disables AAC audio download (highest bitrate MPEG used)\n";
		std::cout << "-pvars                 ; adds the program to PATH variables\n";
		std::cout << "-save                  ; saves applicable arguments to cfg.json\n";

		std::cout << "\n[+] FILES\n";
		std::cout << "-audio-name <name>     ; the name to be given to the downloaded audio file\n";
		std::cout << "-img-name <name>       ; the name to be given to the downloaded cover art\n";
		std::cout << "-img-src <source>      ; the cover art that will be added to the audio (soundcloud track link, image link, or image path)\n";
		
		std::cout << "\n[+] METADATA\n";
		std::cout << "-title <title          ; title property of audio tag (also used for the file name if one is not specified)\n";
		std::cout << "-comment <comment>     ; comment property of the audio tag (automatically adds the upload date, upload description, and tags)\n";
		std::cout << "-artists <artists>     ; contributing artists property of audio tag (defaults to artist metadata or uploader)\n";
		std::cout << "-a-artist <artist>     ; album artist property of audio tag (defaults to album uploader if in an album)\n";
		std::cout << "-album <album>         ; album property of audio tag (defaults to track name or, if track is in an album, the album name)\n";
		std::cout << "-genre <genre>         ; genre property of audio tag (defaults to parsed genre from track)\n";
		std::cout << "-num <track number>    ; track number property of audio tag (defaults to track number in album if track is in an album)\n";
		std::cout << "-year <year>           ; year property of audio tag (defaults to year of upload date)\n";
	}

	static void set_arg_var(PCWSTR src, std::wstring& dst)
	{
		if (src)
		{
			dst = src;
		}
		else
		{
			cfg::f.no_arg_data_provided = true;
		}
	}

	static void set_arg_var(PCWSTR src, UINT& dst)
	{
		if (src)
		{
			try
			{
				dst = std::stoul(src);
			}
			catch (...)
			{
				cfg::f.invalid_data_provided = true;
			}
		}
		else
		{
			cfg::f.no_arg_data_provided = true;
		}
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
			// Parsing argument data

			if (argv[i][0] != '-')
			{
				err::log(L"unrecognized argument \"{}\"", argv[i]);
				return false;
			}

			PCWSTR next_arg = nullptr;
			if (i + 1 < argc) next_arg = argv[i + 1];

			switch (hash_rt(&argv[i][1]))
			{
				// Flag arguments

			case hash(L"save"):    { cfg::f.save_config            = true; break; }
			case hash(L"art"):     { cfg::f.download_art_seperate  = true; break; }
			case hash(L"n-art"):   { cfg::f.disable_art_download   = true; break; }
			case hash(L"audio"):   { cfg::f.download_audio         = true; break; }
			case hash(L"n-audio"): { cfg::f.disable_audio_download = true; break; }
			case hash(L"pvars"):   { cfg::f.add_to_path            = true; break; }
			case hash(L"aac"):     { cfg::f.get_aac_transcoding    = true; break; }
			case hash(L"n-aac"):   { cfg::f.no_aac_transcodings    = true; break; }

				// Config data arguments

			case hash(L"img-name"):   { set_arg_var(next_arg, cfg::g_data.image_file_name); ++i; break; }
			case hash(L"audio-name"): { set_arg_var(next_arg, cfg::g_data.audio_file_name); ++i; break; }
			case hash(L"img-dst"):    { set_arg_var(next_arg, cfg::image_out_dir);          ++i; break; }
			case hash(L"audio-dst"):  { set_arg_var(next_arg, cfg::audio_out_dir);          ++i; break; }
			case hash(L"img-src"):    { set_arg_var(next_arg, cfg::image_src);              ++i; break; }
					
				// Audio tag arguments

			case hash(L"title"):    { set_arg_var(next_arg, cfg::g_data.title);           ++i; break; }
			case hash(L"comment"):  { set_arg_var(next_arg, cfg::g_data.comments);        ++i; break; }
			case hash(L"artists"):  { set_arg_var(next_arg, cfg::g_data.contrib_artists); ++i; break; }
			case hash(L"a-artist"): { set_arg_var(next_arg, cfg::g_data.album_artists);   ++i; break; }
			case hash(L"album"):    { set_arg_var(next_arg, cfg::g_data.album);           ++i; break; }
			case hash(L"genre"):    { set_arg_var(next_arg, cfg::g_data.genre);           ++i; break; }
			case hash(L"num"):      { set_arg_var(next_arg, cfg::g_data.number);          ++i; break; }
			case hash(L"year"):     { set_arg_var(next_arg, cfg::g_data.year);            ++i; break; }

				// Extra

			case hash(L"cid"):
			{
				// Warnings here can be ignored, they're wrong. Stupid compiler.
				cfg::client_id.resize(lstrlenW(next_arg)); // this is really only in its own part cuz its ugly asf
				WideCharToMultiByte(CP_UTF8, 0, next_arg, -1, cfg::client_id.data(), static_cast<int>(cfg::client_id.size()), nullptr, nullptr);
				++i; break;
			}

				// Invalid arguments

			default:
			{
				err::log(L"invalid argument \"{}\"", argv[i]);
				return false;
			}
			}

			// Verifying valid data was provided

			if (cfg::f.no_arg_data_provided)
			{
				err::log(L"no variable provided for argument \"{}\"", argv[i - 1]);
				return false;
			}

			if (cfg::f.invalid_data_provided)
			{
				err::log(L"non-numeric or out of range variable (\"{}\") provided for argument \"{}\"", next_arg, argv[i - 1]);
				return false;
			}
		}

		// Validating arguments

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
			err::log_ex("failed to get SoundLoad path");
			return false;
		}

		cfg::program_dir = std::filesystem::path(program_path).parent_path().wstring();

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

	void add_to_path()
	{
		// Getting 'HKCU/Environment/Path' value

		HKEY hkey = nullptr;
		if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Environment", 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkey) != ERROR_SUCCESS)
		{
			err::log_ex("failed to open HKCU/Environment (ignoring)");
			return;
		}

		DWORD cbSize = 0;
		if (RegGetValueW(hkey, nullptr, L"Path", RRF_NOEXPAND | RRF_RT_REG_EXPAND_SZ, nullptr, nullptr, &cbSize) != ERROR_SUCCESS)
		{
			err::log_ex("failed to query size of HKCU/Environment/Path (ignoring)");
			RegCloseKey(hkey);
			return;
		}

		std::wstring value(cbSize / sizeof(WCHAR), 0);
		if (RegGetValueW(hkey, nullptr, L"Path", RRF_NOEXPAND | RRF_RT_REG_EXPAND_SZ, nullptr, value.data(), &cbSize) != ERROR_SUCCESS)
		{
			err::log_ex("failed to query value of HKCU/Environment/Path (ignoring)");
			RegCloseKey(hkey);
			return;
		}

		// Formatting value

		value.erase(value.find_first_of(L'\0')); // there are multiple null terminators sometimes
		if (value.back() != L';') value.push_back(L';');

		std::transform(value.begin(), value.end(), value.begin(), ::tolower);
		std::transform(cfg::program_dir.begin(), cfg::program_dir.end(), cfg::program_dir.begin(), ::tolower);

		// Setting value

		if (value.find(cfg::program_dir) == std::wstring::npos
		&& (cfg::f.add_to_path || MessageBoxW(nullptr, L"Add SoundLoad to PATH variables?", L"SoundLoad", MB_YESNO | MB_ICONQUESTION) == IDYES))
		{
			value += cfg::program_dir + L';';

			if (RegSetValueExW(hkey, L"Path", 0, REG_EXPAND_SZ, reinterpret_cast<BYTE*>(value.data()), static_cast<DWORD>(value.size() * sizeof(WCHAR))) == ERROR_SUCCESS)
			{
				std::cout << "[!] Added SoundLoad to PATH variables\n";
			}
			else err::log_ex("failed to set HKCU/Environment/Path value (ignoring)");
		}

		RegCloseKey(hkey);
	}
}