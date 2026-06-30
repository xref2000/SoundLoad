#pragma once

// TYPES

struct cfg_format
{
	std::string  cid;
	std::wstring art_out_dir;
	std::wstring track_out_dir;
	bool get_track_audio     = true;
	bool get_track_art       = false;
	bool get_aac_transcoding = false;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(cfg_format, cid, art_out_dir, track_out_dir, get_track_audio, get_track_art, get_aac_transcoding)

// CONFIG DATA

namespace cfg
{
	//
	//// GLOBAL CONFIG VARIABLES
	//

	struct cfg_flags_t
	{
		// 13/16 bits used

		UINT16 add_to_path            : 1; // Adds program to PATH variables
		UINT16 save_config            : 1; // Saves applicable arguments to config (set when -save is used)
		UINT16 save_cid               : 1; // Saves the client ID to config. Set when a client ID is automatically resolved.
		UINT16 download_art_seperate  : 1; // [saveable] downloads track's cover art to its own file
		UINT16 disable_art_download   : 1; // [saveable] [default] disables the DOWNLOAD_ART_SEPERATE flag if it was saved to config
		UINT16 download_audio         : 1; // [saveable] [default] reenables audio downloads if disabled by DISABLE_AUDIO_DOWNLOAD in config
		UINT16 disable_audio_download : 1; // [saveable] disables audio downloads
		UINT16 get_aac_transcoding    : 1; // [saveable] Downloads lossless m4a media transcoding
		UINT16 no_aac_transcodings    : 1; // [saveable] [default] Disables lessless m4a media transcoding downloads
		UINT16 no_link_provided       : 1; // Indicates that no link was provided - used when modifying config without downloading anything
		UINT16 cover_src_is_path      : 1; // Provided source for audio cover art is an image path on the computer
		UINT16 cover_src_is_sc_link   : 1; // Provided source for audio cover art is a SoundCloud link (uses cover art from song/profile)
		UINT16 config_just_created    : 1; // cfg.json was just created
	} inline f = {};

	struct track_data_t
	{
		// Output data

		std::wstring audio_file_name; // Name for downloaded audio. Derived from track title by default.
		std::wstring image_file_name; // Name for downloaded cover art. Derived from track title by default.

		// Audio metadata

		std::wstring title;
		std::wstring comments;
		std::wstring contrib_artists; // contributing artists
		std::wstring album_artists;   // album artist
		std::wstring album;
		std::wstring genre;
		UINT         number = 0U; // track number (for albums)
		UINT         year   = 0U;
	} inline g_data = {};

	inline std::string  client_id;     // SoundCloud client ID
	inline std::wstring audio_out_dir; // Audio download directory
	inline std::wstring image_out_dir; // Cover art download directory
	inline std::wstring image_src;     // Audio cover art source (file path, soundcloud track link, or image link)
	inline std::wstring path_cache;    // Stores the directory of sl.exe or the path of cfg.json

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

	__forceinline bool audio_flags_set(void) noexcept
	{
		return cfg::f.download_audio || cfg::f.disable_audio_download;
	}

	__forceinline bool art_flags_set(void) noexcept
	{
		return cfg::f.download_art_seperate || cfg::f.disable_art_download;
	}

	__forceinline bool aac_flags_set(void) noexcept
	{
		return cfg::f.get_aac_transcoding || cfg::f.no_aac_transcodings;
	}

	__forceinline bool cover_art_only(void) noexcept
	{
		return cfg::f.download_art_seperate && cfg::f.disable_audio_download;
	}
}