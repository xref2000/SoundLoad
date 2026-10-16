#pragma once

enum class cover_types_t : uint8_t
{
	invalid,
	jpg,
	png,
};

struct sc_upload
{
private:

	//
	//// PRIVATE MEMBERS
	//

	Json post_data;

	std::string      streaming_url;
	std::vector<int> track_ids;

	std::wstring art_src; // user-supplied source for cover art
	std::string  artwork_url;
	std::string  artist_pfp_url;

	std::wstring description;
	std::wstring genre;
	std::wstring album;
	std::wstring artist;
	std::wstring title;
	uint32_t year = 0;
	uint32_t num  = 0;

	int id = 0;

	cover_types_t cover_type = cover_types_t::invalid;

	//
	//// PRIVATE METHODS
	//

	bool download_track(void);

	bool download_album(void);

	bool download_cover(void) const;

	bool parse_manifest(const std::string& raw_data, std::deque<std::shared_future<cpr::Response>>& buffer);

	bool get_streaming_url(void);

	bool get_track_ids(void);

	bool get_cover_art(cpr::Response& buffer) const;

	void add_m4a_tag(const std::wstring& path) const;

	void get_misc_metadata(void);

	void create_comments(void);

	void get_album(void);

	bool get_art_and_title(void);

	bool get_artist(void);

	bool get_resolution_data(void);

public:

	//
	//// PUBLIC MEMBERS
	//

	struct
	{
		// 3/8 bits used

		uint8_t err_occured : 1; // an error occured in the constructor
		uint8_t is_track    : 1; // object represents a singular track
		uint8_t is_album    : 1; // object represents an album or playlist
	} f = {};

	//
	//// PUBLIC METHODS
	//

	sc_upload(std::wstring url);

	bool download(void)
	{
		if (cfg::f.seperate_art)
		{
			if (!this->download_cover())
			{
				return false;
			}

			if (cfg::f.no_audio_download)
			{
				std::println("WARNING: only downloaded cover art");
			}
		}

		if (cfg::f.no_audio_download)
		{
			return true;
		}

		return this->f.is_track ? this->download_track() : this->download_album();
	}
};

// Requests a fresh client ID from SoundCloud
bool get_client_id(void);