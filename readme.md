## Rain World Uploader for Linux
#### Version: 0.1.1

This is a simple command line tool to upload Rain World mods to Steam.
If you are on Windows then you can use [RainWorldWorkshopUploader](https://github.com/MatheusVigaro/RainWorldWorkshopUploader) instead.

### Usage
#### Create a mod:
```bash
rain_world_uploader.out /path/to/mod/directory
```
This returns the MOD\_ID for the created mod.

#### Update a mod:
```bash
rain_world_uploader.out MOD_ID /path/to/mod/directory
```
You get the MOD\_ID from creating a mod, or from a Workshop URL ([...]?id=MOD\_ID).

#### Set thumbnail:
```bash
rain_world_uploader.out MOD_ID -thumbnail /path/to/thumbnail.png
```
#### Set tags:
```bash        
rain_world_uploader.out MOD_ID -tags "Tag 1" "Tag 2" ...
```
This overrides existing tags.

### Context

Make sure that `Steam runs in the background`.
This tool does **not** check if the files or metadata are correct.
It just sends all files from a directory to Steam.

You need to specify the `MOD_ID` if the mod exists and the mod's `directory path`.
That is the path that contains the file `modinfo.json`.
You get the MOD\_ID from creating a mod, or from a Workshop URL ([...]?id=MOD\_ID).
 
The SteamAPI requires the file `steam_appid.txt`.
This file is provided along with the executable.
This tool reads the APP\_ID from there as well.
Technically, you upload a mod for any game that supports it by changing the APP\_ID in that file.

### Building from source
You need to download the Steamwork SDK from [Download](https://partner.steamgames.com/downloads/list).
Move the files to the directory `./steamworks_sdk/`.
Create the directory if it does not exist.
Make sure that the path `./steamworks_sdk/public/steam` exists.

Run the script `./build.sh`.

You might get an error that the file `steamclient.so` is missing.
Check the subfolders in `$HOME/.steam/sdk64/` or just search for it.

### Contact
If you have feedback, you can message me on Discord `@schuhbaum` or write an email to SchuhBaum71@gmail.com.

### License
See the file LICENSE-MIT.

### Changelog
v0.1.1:
- Fixed a bug where the CPU would run at 100% while waiting.

v0.1.0:
- Initial release.
