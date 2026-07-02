# SoundLoad
The goal of this project is to help SoundCloud users save their favorite songs 
with as much data as possible preserved, and to simplify the process of porting 
songs to other platforms, which can be very tedious normally. I'm aware that 
the people using this will have little to no technical experience, so I've explained 
exactly how to use the program in a way that anyone with a browser and ChatGPT 
should be able to figure out.

If you prefer porting music to Spotify or Apple Music, you should not enable AAC 
downloads, as neither platforms will play them. The difference in bitrate is
negligible anyway, so this should only be used if you care about archiving.

Any programmer seeing this will surely wonder why I would write this in C++ when 
golang is so much easier. I did it because this is quite different from every other 
C++ project I've worked on, so I thought it would be good practice.

## Installation & usage
If you know how to use command line programs you can skip this. If you don't, 
follow these steps to use the program:

- Look for the "Releases" tab on the GitHub repository
- You should see a release with "Latest" in green text, click on it and download `sl.zip`
- Find and extract `sl.zip` in your file explorer and open the folder containing `sl.exe`
- Click on any empty space in the bar at the top of your file explorer that shows your directory. 
  Erase the directory, type "cmd", and hit enter.
- Type `sl -?` and hit enter to see a list of all available features, or reference the table below

If you allow the program to add itself to PATH variables (it will ask), you 
will not have to open the files directory in the future, instead you will be 
able to quickly open cmd and use the program immediately.

## Arguments (case-insensitive)

| Argument      |   Short  | Description                                          | Saveable? | Album/Playlist? |
| ------------- | :------: | ---------------------------------------------------- | :-------: | :-------------: |
| `-cid`        |     —    | SoundCloud client ID                                 |     ✅    |        ✅      |
| `-pvars`      |     —    | Adds program to user PATH variables                  |     ❌    |        ✅      |
| `-save`       |   `-s`   | Saves applicable arguments to `cfg.json`             |     ❌    |        ✅      |
| `-img-name`   | `-iname` | Cover art file name                                  |     ❌    |        ✅      |
| `-img-dst`    |  `-idst` | Cover art output directory                           |     ✅    |        ✅      |
| `-img-src`    |  `-isrc` | Cover art source (path, SoundCloud link, image link) |     ❌    |        ✅      |
| `-art`        |     —    | Independent cover art download                       |     ✅    |        ✅      |
| `-n-art`      |     —    | Disable independent cover art download               |     ✅    |        ✅      |
| `-audio-name` | `-aname` | Audio file name                                      |     ❌    |        ✅      |
| `-audio-dst`  |  `-adst` | Audio output directory                               |     ✅    |        ✅      |
| `-audio`      |     —    | Enables audio downloads                              |     ✅    |        ✅      |
| `-n-audio`    |     —    | Disables MP3 downloads                               |     ✅    |        ✅      |
| `-aac`        |     —    | Enables lossless downloads                           |     ✅    |        ✅      |
| `-n-aac`      |     —    | Disables lossless downloads                          |     ✅    |        ✅      |
| `-title`      |   `-t`   | Audio tag title                                      |     ❌    |        ❌      |
| `-comment`    |   `-c`   | Audio tag comment                                    |     ❌    |        ✅      |
| `-artists`    |   `-a`   | Audio tag contributing artists                       |     ❌    |        ❌      |
| `-a-artist`   |   `-aa`  | Audio tag album artist                               |     ❌    |        ✅      |
| `-album`      |   `-al`  | Audio tag album                                      |     ❌    |        ✅      |
| `-genre`      |   `-g`   | Audio tag genre                                      |     ❌    |        ✅      |
| `-num`        |   `-n`   | Audio tag track number                               |     ❌    |        ❌      |
| `-year`       |   `-y`   | Audio tag year                                       |     ❌    |        ✅      |


## Examples

### Setting basic config info
By running this, the audio output dir and cover art output dir will be saved to cfg.json.
```
c:>sl -adst c:/music -idst c:/art -s
```

### Downloading a song
This will download the song, name the audio file "Extra Stixx", and set the contributing artists to "Pook G, Lul Jody".
Other metadata will be automatically scraped from the page, but can of course be set manually.
```
c:>sl https://soundcloud.com/fat-kid-915108395/exrta-stixxs-ft-pook-g-lul -aname "Extra Stixx" -a "Pook G, Lul Jody"
```

### Downloading an album
Songs are downloaded in order of last to first, with track numbers automatically parsed. If you want more 
control over each track in the album, download them independently.
```
c:>sl https://soundcloud.com/axxturel/sets/s-kkkult-s-kkkult-s-kkkkult
```

### Downloading cover art
By using the `-art` arg, you tell the program to seperately download the cover art from the track. It 
should be noted that this option can be saved to cfg.json. This is useful for anyone archiving 
underground music, where cover art is often changed or lost to time. The `-n-audio` arg is also used 
and saved, preventing the track itself from being downloaded. Cover art will be embedded into the 
tracks metadata (if one is downloaded) in the highest available quality no matter what.
```
c:>sl https://soundcloud.com/sellasouls/bdayy-sexxx -n-audio -art -s
```

## Extensibility
I understand that it's unlikely anyone using this has experience with C++. If that's you, 
feel free to open an issue on this repository or reach out via git@vmcall.dev and tell me 
what you'd like added and I will add it. That being said, the source code is structured in 
such a way that makes adding new metadata fields extremely simple. First, go to 
`https://api-v2.soundcloud.com/resolve?url=put_link_here&client_id=your_cid` and take note 
of any fields you would like to archive. You then go to `sc_api.cpp/sc_upload::sc_upload` 
and use the `add_comment` lambda, which takes just one simple line of code. Below is a 
snippet from the source code with extra comments added explaining exactly how it works 
and showing multiple examples for those who would like to do it on their own but don't 
have experience with C++, though if this is you I strongly suggest reaching out to me so 
I can add it for you, because compiling projects like this from source would probably be a 
nightmare for someone who has no clue what they're doing.
```C++
// Don't worry about the confusing syntax here, you don't touch this block of code at all
auto add_comment = [this, &post_data, &temp](PCWSTR label, PCSTR value)
	{
		mb_to_wide(post_data.value(value), temp);

		if (!temp.empty()) // only continues if the desired field exists and contains data
		{
			if (!this->description.empty())
			{
				// If this isn't the first line to be inserted into the comment field, 
				// it adds two lines of white space for seperation
				this->description += L"\n\n";
			}

			// Appends the desired metadata value to the label. This results in 
			// something like "Original title: example song title"
			this->description += label + temp;

			temp.clear();
		}
	};

// This is where you may use the code above. As I already explained, you simply 
// choose a field from the SoundCloud API that you'd like stored in the comments, 
// choose a description for the field, and pass them to add_comment the same way 
// that I've shown below.

add_comment(L"Upload date: ",          "created_at");
add_comment(L"Original title: ",       "title");
add_comment(L"Original description: ", "description");
add_comment(L"Original tags: ",        "tag_list");
```

## PATH variables
Adding the program to your PATH variables will basically tell the command prompt 
what directories to check when you enter a command. For example, if you type `sl.exe` 
into cmd, it will check your working directory aswell as all directories specified 
in your PATH vars to try and find where `sl.exe` exists. This is a huge time saver 
as it allows you to quickly open cmd and use the program no matter your working 
directory. When you run the program for the first time, you'll be asked whether 
or not you'd like to add the program to your PATH vars, but you can do it at a later 
time with the `-pvars` argument. Because you're adding the programs directory, 
any other files in that directory will also be resolved by cmd, so make sure you 
keep the program and it's dependencies in it's own directory to avoid name conflicts.

## Client IDs
A client ID is a string that must be appended to certain requests in order for 
them to succeed. The program will automatically resolve and save a client ID, 
but in the case that it fails, you can provide a client ID manually like this:

- Open browser dev tools (`ctrl+shift+i`)
- Go to the network tab
- Go to [SoundCloud](https://soundcloud.com)
- Filter URLs with "client_id="
- Find a request with a client ID in it
- Pass that ID to `sl.exe` as the value for the `-cid` argument

That being said, please open an issue on this repository if it's failing to resolve 
a client ID automatically, as failure is indicative of a SoundCloud update.

## Notes
- Windows doesn't parse metadata properly. VLC doesn't have this issue, so check there if something 
  seems out of place.
- `nlohmann::json::value` has multiple modifications in this repo - check `pch.hpp` for more info.
- Unicode characters are supported, though they won't be displayed properly on command line due to 
  the limited character set. You can safely ignore that and paste any unicode character, it will save as expected.
- The following characters will be replaced with an underscore in file names: `< > : " \ | ? *`
- Go+ songs cannot be downloaded (may work if you use a CID generated by a Go+ account, not tested).
- `cfg.json` should not be modified manually for now. The JSON library I use doesn't provide a proper 
  method of storing unicode strings which results in inefficient path storage. This will be fixed later.