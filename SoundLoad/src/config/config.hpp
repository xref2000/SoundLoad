#pragma once

// TYPES

struct cfg_format
{
	std::string  cid;
	std::wstring art_out_dir;
	std::wstring track_out_dir;
	bool get_track_audio = true;
	bool get_track_art   = false;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(cfg_format, cid, art_out_dir, track_out_dir, get_track_audio, get_track_art)

// CONFIG DATA

namespace cfg
{
	//
	//// GLOBAL CONFIG VARIABLES
	//

	struct cfg_flags_t
	{
		// 11/16 bits used

		uint16_t add_to_path       : 1; // Adds program to PATH variables
		uint16_t save_cfg          : 1; // Saves applicable arguments to config (set when -save is used)
		uint16_t save_cid          : 1; // Saves the client ID to config. Set when a client ID is automatically resolved.
		uint16_t seperate_art      : 1; // [saveable] downloads track's cover art to its own file
		uint16_t no_seperate_art   : 1; // [saveable] [default] disables the seperate_art flag if it was saved to config
		uint16_t download_audio    : 1; // [saveable] [default] reenables audio downloads if disabled by no_audio_download in config
		uint16_t no_audio_download : 1; // [saveable] disables audio downloads
		uint16_t no_link_provided  : 1; // Indicates that no link was provided - used when modifying config without downloading anything
		uint16_t cover_src_path    : 1; // Provided source for audio cover art is an image path on the computer
		uint16_t cover_src_sc_link : 1; // Provided source for audio cover art is a SoundCloud link (uses cover art from song/profile)
		uint16_t fresh_cfg         : 1; // cfg.json was just created
	} inline f = {};

	struct track_data_t
	{
		// Output data

		std::wstring audio_name; // Name for downloaded audio. Derived from track title by default.
		std::wstring image_name; // Name for downloaded cover art. Derived from track title by default.

		// Audio metadata

		std::wstring title;
		std::wstring comments;
		std::wstring contrib_artists; // contributing artists
		std::wstring album_artists;   // album artist
		std::wstring album;
		std::wstring genre;
		std::wstring number; // track number (for albums)
		std::wstring year;
	} inline g_data = {};

	inline std::string  client_id;  // SoundCloud client ID
	inline std::wstring audio_dir;  // Audio download directory
	inline std::wstring image_dir;  // Cover art download directory
	inline std::wstring image_src;  // Audio cover art source (file path, soundcloud track link, or image link)
	inline std::wstring path_cache; // Stores the directory of sl.exe or the path of cfg.json

	//
	//// FORWARD DECLARATIONS
	//

	bool parse_arguments(int argc, wchar_t* argv[]);

	void read_config(const cfg_format& data);

	void save_config(cfg_format& data);

	void add_to_path(void);

	//
	//// HELPERS
	//

	__forceinline bool audio_flags_set(void)
	{
		return cfg::f.download_audio || cfg::f.no_audio_download;
	}

	__forceinline bool art_flags_set(void)
	{
		return cfg::f.seperate_art || cfg::f.no_seperate_art;
	}

	__forceinline bool cover_art_only(void)
	{
		return cfg::f.seperate_art && cfg::f.no_audio_download;
	}
}