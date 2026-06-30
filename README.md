# SoundLoad
The goal of this project is to help SoundCloud users save their favorite songs 
with as much data as possible preserved. I made this to simplify the process of 
porting SoundCloud songs to other platforms, which can be very tedious normally. 
If you value the use of local files, I strongly suggest apple music. They have an 
extremely simple and well featured local files system compared to the shitty 
alternative that spotify provides, which doesn't even work for most people.

If you prefer porting music to Spotify or Apple Music, you should not enable AAC 
downloads, as neither platforms will play them. The difference in bitrate is
negligible anyway, so this should only be used if you care about archiving.

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
and saved, preventing the track itself from being downloaded.
```
c:>sl https://soundcloud.com/sellasouls/bdayy-sexxx -n-audio -art -s
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
- In the `comments` property of downloaded audio tags, the program will store the exact timestamp 
  of the upload and the the original title/description/tags. You can easily add on to this list 
  in `site_api.cpp/sc_upload::sc_upload()` using the `add_comment` lambda.
- The following characters will be replaced with an underscore in file names: `< > : " \ | ? *`
- Go+ songs cannot be downloaded (may work if you use a CID generated by a Go+ account, not tested).
- `cfg.json` must not be modified manually for now. The json library I'm using doesn't have std::wstring
  support, causing it to store such types in extremely stupid ways. Only touch if you know what you're doing.
  This will be fixed later.