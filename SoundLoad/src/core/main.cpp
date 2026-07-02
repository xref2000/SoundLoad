#include "pch.hpp"
#include "../config/config.hpp"
#include "../site_api/site_api.hpp"

static void fix_path(std::wstring& path)
{
	if (!path.empty())
	{
		std::replace(path.begin(), path.end(), L'\\', L'/');
		if (path.back() != L'/') path.push_back(L'/');
	}
}

static bool save_config(cfg_format& cfg_data_raw)
{
	if (cfg::f.save_config || cfg::f.save_cid)
	{
		if (cfg::f.save_config)
		{
			cfg::save_config(cfg_data_raw);
		}
		else
		{
			cfg_data_raw.cid = cfg::client_id;
		}

		std::fstream cfg_file(cfg::path_cache, std::ios::out | std::ios::trunc);
		if (cfg_file.fail())
		{
			err::log("failed to open/create cfg.json");
			return false;
		}

		try
		{
			cfg_file << Json(cfg_data_raw).dump();
		}
		catch (...)
		{
			cfg_file.close();
			err::log("failed to save config");
			return false;
		}

		cfg_file.close();
	}

	return true;
}

static bool read_config(cfg_format& cfg_data_raw)
{
	if (std::filesystem::exists(cfg::path_cache))
	{
		std::fstream cfg_file(cfg::path_cache, std::ios::in);
		if (cfg_file.fail())
		{
			err::log("failed to open cfg.json");
			return false;
		}

		try
		{
			Json cfg_data_json;
			cfg_file >> cfg_data_json;
			cfg_data_raw = cfg_data_json.get<cfg_format>();
		}
		catch (...)
		{
			cfg_file.close();
			err::log("failed to read config");
			return false;
		}

		cfg_file.close();
		cfg::read_config(cfg_data_raw);
	}
	else
	{
		cfg::f.config_just_created = true;
	}

	return true;
}

static bool init_program(int argc, wchar_t* argv[], cfg_format& raw_cfg)
{
	if (!cfg::parse_arguments(argc, argv))
	{
		return false;
	}

	const size_t old_size = cfg::path_cache.size();
	cfg::path_cache += L"\\cfg.json";

	if (!read_config(raw_cfg))
	{
		return false;
	}

	if (!cfg::f.no_link_provided && cfg::client_id.empty() && !get_client_id())
	{
		err::log("no client ID provided/resolved");
		return false;
	}

	if (cfg::f.add_to_path || cfg::f.config_just_created)
	{
		cfg::path_cache[old_size] = L'\0';
		cfg::add_to_path(); // failure is ignored as its non-vital to primary functionality
		cfg::path_cache[old_size] = L'\\';
	}

	if (cfg::f.no_link_provided)
	{
		if (save_config(raw_cfg))
		{
			std::cout << "\n[!] INPUT HANDLED\n";
		}

		return false;
	}

	return true;
}

int wmain(int argc, wchar_t* argv[])
{
	cfg_format raw_cfg = {};
	if (!init_program(argc, argv, raw_cfg))
	{
		system("pause"); // half the world would give up when double clicking does nothing
		return 1;
	}

	fix_path(cfg::audio_out_dir);
	fix_path(cfg::image_out_dir);

	sc_upload post(argv[1]);
	if (post.f.error_occured || !post.download())
	{
		return 1;
	}

	std::cout << "\n[!] DOWNLOAD(s) COMPLETE\n";

	save_config(raw_cfg); // this is done after downloads complete incase request_failed had to get a new CID
	return 0;
}