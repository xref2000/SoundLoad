#include "pch.hpp"
#include "../config/config.hpp"
#include "site_api.hpp"

//
//// HELPER FUNCTIONS
//

static void mb_to_wide(const std::string& src, std::wstring& dst)
{
	if (!src.empty())
	{
		dst.resize(src.size());
		MultiByteToWideChar(CP_UTF8, 0, src.c_str(), -1, dst.data(), static_cast<int>(dst.size()));
	}
}

static size_t read_file(const std::wstring& path, char** buffer)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (file.fail())
	{
		err::log(L"failed to open file \"{}\"", path);
		return 0;
	}

	const size_t sz = file.tellg();
	char* raw_data = new char[sz];

	file.seekg(0, std::ios::beg);
	file.read(raw_data, sz);
	file.close();

	*buffer = raw_data;
	return sz;
}

bool request_failed(cpr::Response& r)
{
	// This function will return true is the request succeeded. Otherwise it will attempt 
	// to get a fresh CID and retry the request with it. For that reason, this function 
	// should only be used on requests that included a CID.

	if (r.status_code == 200)
	{
		return false;
	}

	if (cfg::f.save_cid || !get_client_id())
	{
		err::log_net(r);
		return true;
	}

	cpr::Url url = r.url.str();

	constexpr char cid_str[] = "client_id=";
	size_t pos = url.str().find(cid_str);
	if (pos == std::string::npos)
	{
		err::log_net(r);
		return true;
	}

	const_cast<std::string&>(url.str()).erase((pos + _countof(cid_str)) - 1);
	url += cfg::client_id;

	r = cpr::Get(url);
	if (r.status_code != 200)
	{
		err::log_net(r);
		return true;
	}
	
	return false;
}

static bool resolve_post(std::wstring& url, Json& buffer)
{
	// Erasing tracking data from link

	if (url.find(L'?') != std::string::npos)
		url.erase(url.find_first_of(L'?'));
	
	// Resolving post

	std::string mb_url(url.size(), 0);
	WideCharToMultiByte(CP_UTF8, 0, url.c_str(), -1, mb_url.data(), static_cast<int>(mb_url.size()), nullptr, nullptr);

	cpr::Response r = cpr::Get(cpr::Url{ "https://api-v2.soundcloud.com/resolve?url=" + mb_url + "&client_id=" + cfg::client_id });
	if (request_failed(r)) return false;

	buffer = Json::parse(r.text);
	return true;
}

bool get_client_id(void)
{
	auto handle_parsing_err = [](void)
		{
			if (cfg::client_id.empty())
			{
				err::log("failed to automatically resolve client ID, please provide one using the -cid command");
				return false;
			}
			else
			{
				err::warn("failed to resolve client ID, falling back to user provided ID");
				return true;
			}
		};

	cpr::Response r = cpr::Get(cpr::Url{ "https://soundcloud.com/" });
	if (r.status_code != 200)
	{
		err::log_net(r);
		return false;
	}

	constexpr char id_string[] = "\"data\":{\"id\":\"";
	const size_t id_pos = r.text.find(id_string);
	if (id_pos == std::string::npos || r.text.find(id_string, id_pos + 1) != std::string::npos)
	{
		return handle_parsing_err();
	}

	const size_t offset = (id_pos + _countof(id_string)) - 1;
	const size_t end_pos = r.text.find('\"', offset);
	if (end_pos == std::string::npos)
	{
		return handle_parsing_err();
	}

	cfg::client_id = r.text.substr(offset, end_pos - offset);
	cfg::f.save_cid = 1; // CID is automatically saved to cfg.json to minimize the amount of requests that must be made in the future
	return true;
}

//
//// MEMBER FUNCTIONS
//

bool sc_upload::get_cover_link(std::string& buffer) const
{
	if (cfg::f.cover_src_is_sc_link)
	{
		Json post_data;
		if (!resolve_post(cfg::image_src, post_data))
		{
			return false;
		}

		buffer = post_data.value("artwork_url");
		if (buffer.empty())
		{
			return false;
		}

		buffer = std::regex_replace(buffer, std::regex("large."), "original.");
		return true;
	}
	else
	{
		buffer.resize(this->art_src.size());
		WideCharToMultiByte(CP_UTF8, 0, this->art_src.c_str(), -1, buffer.data(), buffer.size(), nullptr, nullptr);
		return true;
	}
}

void sc_upload::add_m4a_tag(const std::wstring& path) const
{
	// Opening file

	TagLib::MP4::File file(path.c_str());
	TagLib::MP4::Tag* const tag = file.tag();

	// Saving basic metadata

	tag->setAlbum(this->album);

	tag->setTitle(this->title);

	tag->setArtist(this->artist);

	tag->setGenre(this->genre);

	tag->setComment(this->description);

	tag->setYear(this->year);

	// Getting cover art

	if (cfg::f.cover_src_is_path)
	{
		char* raw_image;
		const size_t sz = read_file(cfg::image_src, &raw_image);
		if (!sz)
		{
			file.save();
			return;
		}

		TagLib::MP4::CoverArtList cover_list;
		cover_list.append({ TagLib::MP4::CoverArt::Unknown, { raw_image, static_cast<UINT>(sz) } });
		delete[] raw_image;

		tag->setItem("covr", TagLib::MP4::Item(cover_list));
	}
	else
	{
		std::string cover_url;
		if (!this->get_cover_link(cover_url))
		{
			file.save();
			return;
		}

		cpr::Response r = cpr::Get(cpr::Url{ cover_url });
		if (request_failed(r))
		{
			file.save();
			return;
		}

		TagLib::MP4::CoverArtList cover_list;
		cover_list.append({ TagLib::MP4::CoverArt::Unknown, { r.text.data(), static_cast<UINT>(r.text.size())} });

		tag->setItem("covr", TagLib::MP4::Item(cover_list));
	}

	file.save();
}

void sc_upload::add_mp3_tag(const std::wstring& path) const
{
	// Creating ID3v2 tag

	TagLib::MPEG::File file(path.c_str());
	TagLib::ID3v2::Tag* const tag = file.ID3v2Tag(true);

	// Setting metadata

	tag->setAlbum(this->album);

	tag->setTitle(this->title);

	tag->setArtist(this->artist);

	tag->setGenre(this->genre);

	tag->setComment(this->description);
	
	tag->setYear(this->year);

	// Setting cover art

	auto cover = new TagLib::ID3v2::AttachedPictureFrame;

	if (cfg::f.cover_src_is_path)
	{
		char* raw_image;
		const size_t sz = read_file(cfg::image_src, &raw_image);
		if (!sz)
		{
			file.save();
			delete cover;
			return;
		}

		cover->setPicture({ raw_image, static_cast<UINT>(sz) });
		delete[] raw_image;
	}
	else
	{
		std::string cover_url;
		if (!this->get_cover_link(cover_url))
		{
			file.save();
			delete cover;
			return;
		}

		cpr::Response r = cpr::Get(cpr::Url{ cover_url });
		if (request_failed(r))
		{
			file.save();
			delete cover;
			return;
		}

		cover->setPicture({ r.text.data(), static_cast<UINT>(r.text.size()) });
	}
	
	tag->addFrame(cover);
	file.save();
}

bool sc_upload::get_track_ids(const Json& data)
{
	const auto tracks = data.value("tracks", Json{});
	if (tracks.empty())
	{
		err::log("failed to parse track ID's");
		return false;
	}

	for (size_t i = tracks.size() - 1; i; --i)
	{
		track_ids.push_back(tracks[i].value("id", 0));
	}

	return true;
}

bool sc_upload::parse_manifest(const std::string& raw_data, std::string& buffer) const
{
	// This is used to download .m3u/.m3u8 files. Both file types are similar, 
	// with each containing an array of links that lead to file segments, which 
	// you must append to eachother in the order of which the links are provided.
	// Any HLS media transcoding will provide this type of file, and each link will 
	// expire relatively quick, so you must programatically download them in the 
	// case of SoundCloud.

	std::istringstream iss(raw_data);
	std::string line;

	while (std::getline(iss, line))
	{
		if (this->f.is_m4a_media && line.starts_with("#EXT-X-MAP:URI="))
		{
			line.erase(0, line.find_first_of('\"') + 1);
			line.pop_back();
		}
		else if (line.starts_with("#EXTINF"))
		{
			std::getline(iss, line);
		}
		else continue;

		cpr::Response r = cpr::Get(cpr::Url{ line });
		if (request_failed(r)) return false;

		buffer += r.text;
	}

	if (buffer.empty())
	{
		err::log("failed to parse HLS manifest");
		return false;
	}

	return true;
}

bool sc_upload::get_streaming_url(const Json& json)
{
	auto list = json.value("media", Json{});
	if (list.empty())
	{
		err::log("'media' property not found in metadata");
		return false;
	}

	list = list.value("transcodings", Json{});
	if (list.empty())
	{
		err::log("'transcodings' property not found in metadata");
		return false;
	}

	bool found = false, is_hls = false, progressive_found = false;

	for (const auto& transcoding : list)
	{
		// AAC transcodings are lossless, and provide .m4a files rather than .mp3.
		// Unfortunately, Spotify can't play these files. 

		if (transcoding["preset"] != "aac_160k")
		{
			if (!progressive_found)
			{
				const auto format = transcoding["format"];
				const bool hls = format["protocol"] == "hls";

				if (format["protocol"] == "progressive" || format["mime_type"] == "audio/mpeg")
				{
					is_hls = hls;

					if (!hls)
					{
						progressive_found = true;
					}

					this->streaming_url = transcoding["url"].get<std::string>() + "?client_id=" + cfg::client_id;
					found = true;
				}
			}
		}
		else if (cfg::f.get_aac_transcoding)
		{
			this->streaming_url  = transcoding["url"].get<std::string>() + "?client_id=" + cfg::client_id;
			this->f.is_m4a_media = true;

			found = true;
			break;
		}
	}

	if (!found)
	{
		err::log("failed to locate valid media transcoding");
		return false;
	}

	if (is_hls)
	{
		this->f.is_hls_mpeg = true;
	}

	return true;
}

bool sc_upload::download_track(void)
{
	// Downloading track (MP3 or HLS)

	cpr::Response r = cpr::Get(cpr::Url{ this->streaming_url });
	if (request_failed(r)) return false;

	r = cpr::Get(cpr::Url{ Json::parse(r.text)["url"].get<std::string>()});
	if (request_failed(r)) return false;

	this->streaming_url.clear();
	
	// Finalizing MP3 download

	const char* raw_audio = nullptr;
	size_t audio_size = 0;

	// This must be outside of the if-else scope below or else raw_mp3 will hold a dangling pointer for HLS downloads.
	// If one std::string buffer was used for HLS and progressive it would require copying the entire mp3 across memory.
	std::string raw_hls_result;

	if (this->f.is_hls_mpeg || this->f.is_m4a_media)
	{
		if (!this->parse_manifest(r.text, raw_hls_result))
			return false;

		raw_audio  = raw_hls_result.data();
		audio_size = raw_hls_result.size();
	}
	else
	{
		raw_audio  = r.text.data();
		audio_size = r.text.size();
	}

	// Writing MP3 to disk

	const std::wstring path = cfg::audio_out_dir + std::regex_replace(this->title, std::wregex(L"[<>:\"/\\|?*]"), L"_") + (this->f.is_m4a_media ? L".m4a" : L".mp3");

	std::ofstream mp3_file(path, std::ios::binary | std::ios::trunc);
	if (mp3_file.fail())
	{
		err::log(L"failed to create mp3 file at \"{}\"", path);
		return false;
	}

	mp3_file.write(raw_audio, audio_size);
	mp3_file.close();

	// Adding ID3v2 tag

	if (this->f.is_m4a_media)
	{
		this->add_m4a_tag(path);
	}
	else
	{
		this->add_mp3_tag(path);
	}

	return true;
}

bool sc_upload::download_album(void)
{
	return true;
}

bool sc_upload::download_cover(void) const
{
	cpr::Response r = cpr::Get(cpr::Url{ this->artwork_url });
	if (r.status_code != 200) return false;

	const std::wstring& file_name = cfg::g_data.image_file_name.empty() ? this->title : cfg::g_data.image_file_name;
	const std::wstring path = cfg::image_out_dir + std::regex_replace(file_name, std::wregex(L"[<>:\"/\\|?*]"), L"_") + L".jpg";

	std::ofstream file(path, std::ios::binary | std::ios::trunc);
	if (file.fail())
	{
		err::log(L"failed to open/create \"{}\"", path);
		return false;
	}

	file.write(r.text.data(), r.text.size());
	file.close();

	return true;
}

sc_upload::sc_upload(std::wstring url)
{
	// Resolving post

	Json post_data;
	if (!resolve_post(url, post_data))
	{
		this->f.error_occured = true;
		return;
	}

	// Getting artwork URL & post title
	
	{
		// Getting title

		if (cfg::g_data.title.empty())
		{
			mb_to_wide(post_data.value("title"), this->title);
			this->title.resize(lstrlenW(this->title.c_str()));
		}
		else
		{
			this->title = cfg::g_data.title;
		}

		// Getting artwork URL

		this->artwork_url = post_data.value("artwork_url", post_data.value("avatar_url"));
		if (!this->artwork_url.empty())
		{
			this->artwork_url = std::regex_replace(this->artwork_url, std::regex("large."), "original.");
		}

		// Getting MP3 artwork source

		if (cfg::image_src.empty())
		{
			mb_to_wide(this->artwork_url, this->art_src);
		}
		else
		{
			this->art_src = cfg::image_src;
		}

		if (cfg::cover_art_only())
		{
			return;
		}
	}

	// Getting album (order: cfg::g_track_data.album -> first existing album via SoundCloud API -> title)

	{
		if (cfg::g_data.album.empty())
		{
			const cpr::Response r = cpr::Get(cpr::Url{ "https://api-v2.soundcloud.com/tracks/" + std::to_string(post_data["id"].get<int>()) + "/albums?client_id=" + cfg::client_id });

			if (r.status_code == 200)
			{
				const Json album_data = Json::parse(r.text)["collection"];

				if (!album_data.empty())
				{
					mb_to_wide(album_data[0].value("title"), this->album);
				}
			}

			if (this->album.empty())
			{
				this->album = this->title;
			}
		}
		else
		{
			this->album = cfg::g_data.album;
		}
	}

	// Creating comments

	{
		// Specified comments

		this->description = cfg::g_data.comments;

		// Extra comments

		std::wstring temp;

		auto add_comment = [this, &post_data, &temp](PCWSTR label, PCSTR value)
			{
				mb_to_wide(post_data.value(value), temp);

				if (!temp.empty())
				{
					if (!this->description.empty())
					{
						this->description += L"\n\n";
					}

					this->description += label + temp;

					temp.clear();
				}
			};

		add_comment(L"Upload date: ",          "created_at");
		add_comment(L"Original title: ",       "title");
		add_comment(L"Original description: ", "description");
		add_comment(L"Original tags: ",        "tag_list");
	}
	
	// Getting genre and year

	{
		// Getting genre

		if (cfg::g_data.genre.empty())
		{
			mb_to_wide(post_data.value("genre"), this->genre);
		}
		else
		{
			this->genre = cfg::g_data.genre;
		}

		// Getting year

		if (cfg::g_data.year)
		{
			this->year = cfg::g_data.year;
		}
		else
		{
			this->year = std::stoul(post_data["created_at"].get<std::string>());
		}
	}

	// Getting artist

	{
		if (cfg::g_data.contrib_artists.empty())
		{
			std::string mb_artist;

			if ((mb_artist = post_data.value("publisher_metadata", Json{}).value("artist")).empty() 
			&&  (mb_artist = post_data.value("user",               Json{}).value("username")).empty())
			{
				err::log("failed to get publisher_metadata.artist or user.username");
				this->f.error_occured = true;
				return;
			}

			mb_to_wide(mb_artist, this->artist);
		}
		else
		{
			this->artist = cfg::g_data.contrib_artists;
		}
	}
	
	// Getting upload type, track ID(s), and streaming url (if applicable)

	{
		const std::string kind = post_data.value("kind");
		if (kind.empty())
		{
			err::log("failed to get 'kind' field from metadata");
			this->f.error_occured = true;
			return;
		}

		if (kind[0] == 't')
		{
			this->id = post_data.value("id", 0);
			this->f.is_track = true;

			if (!get_streaming_url(post_data))
			{
				this->f.error_occured = true;
			}
		}
		else
		{
			this->f.is_album = true;

			if (!get_track_ids(post_data))
			{
				this->f.error_occured = true;
			}
		}
	}
}