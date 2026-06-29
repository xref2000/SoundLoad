#pragma once

struct sc_upload
{
private:

	//
	//// PRIVATE MEMBERS
	//

	std::string      streaming_url;
	std::vector<int> track_ids;

	std::wstring art_src;
	std::string  artwork_url;
	std::string  artist_pfp_url;

	std::wstring description;
	std::wstring genre;
	std::wstring album;
	std::wstring artist;
	std::wstring title;
	UINT         year = 0u;
	UINT         num  = 0u;

	int id = 0;

	//
	//// PRIVATE METHODS
	//

	bool download_track(void);

	bool download_album(void);

	bool download_cover(void) const;

	bool parse_manifest(const std::string& raw_data, std::string& buffer) const;

	bool get_streaming_url(const Json& data);

	bool get_track_ids(const Json& data);

	bool get_cover_link(std::string& buffer) const;

	void add_m4a_tag(const std::wstring& path) const;

	void add_mp3_tag(const std::wstring& path) const;

public:

	//
	//// PUBLIC MEMBERS
	//

	struct
	{
		// 5/8 bits used

		UINT8 error_occured : 1; // An error occured in the constructor
		UINT8 is_track      : 1; // Object represents a singular track
		UINT8 is_album      : 1; // Object represents an album or playlist
		UINT8 is_m4a_media  : 1; // Lossless media transcoding (.m3u8 -> .m4a)
		UINT8 is_hls_mpeg   : 1; // HLS media transcoding (.m3u -> .mp3)
	} f = {};

	//
	//// PUBLIC METHODS
	//

	sc_upload(std::wstring url);

	bool download(void)
	{
		if (cfg::f.download_art_seperate)
		{
			if (!this->download_cover())
			{
				return false;
			}

			if (cfg::f.disable_audio_download)
			{
				std::cout << "WARNING: only downloaded cover art\n";
			}
		}

		if (cfg::f.disable_audio_download)
		{
			return true;
		}

		return this->f.is_track ? this->download_track() : this->download_album();
	}
};

// Requests a fresh client ID from SoundCloud
bool get_client_id(void);