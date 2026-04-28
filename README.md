# Dynamic Dialogue Replacer

SKSE plugin that allows for runtime replacement of dialogue subtitles and voice files. 

For documentation on how to use this plugin, see the [wiki](https://github.com/KrisV-777/Dynamic-Dialogue-Replacer/wiki).  
The Nexus Mods mirror can be found [here](https://www.nexusmods.com/skyrimspecialedition/mods/135618).

## Requirements
* [xmake](https://xmake.io/#/)
	* Add this to your `PATH`
* [PowerShell](https://github.com/PowerShell/PowerShell/releases/latest)
* [Visual Studio Community 2022](https://visualstudio.microsoft.com/)
	* Desktop development with C++

## Building

### Clone
```
https://github.com/KrisV-777/Dynamic-Dialogue-Replacer.git
cd Dynamic-Dialogue-Replacer
git submodule update --init --recursive
```

### Configure
Before building, the following values have to be provided:
1. **papyrus_path**: The path to your Papyrus Compiler
2. **papyrus_gamesource**: The path to your (game/SKSE) Papyrus sources

These can be provided either:
1. Environment variables (optionally loaded from `.env`)
   - See `.env.example` for variables/descriptions.
   - Copy to `.env` and edit the values.
   - Run `xmake f -c` after making changes to load the new values.
   - Alternate files can be loaded with `xmake f -c --dotenv=.env.other`

2. Command-line configure
   - Set options when configuring with `xmake f`:
```sh
xmake f -m release \
	--papyrus_path="path\to\Papyrus Compiler" \
	--papyrus_gamesource="path\to\Skyrim Special Edition\Data"
```

### Build

```sh
xmake
```

### Install

If `install_path` and `auto_install` are configured, files will be automatically coppied to `install_path` after a successful build. Otherwise install can be run manually using:
```sh
xmake install -o INSTALLDIR
```

## Packaging
Package the project into a `.7z` distribution using:
```
xmake pack
```
The file will be located in `build\xpack\`.

## Papyrus Project Generation

Generate a papyrus project file for IDE integration using:
```sh
xmake papyrus.project papyrus
```
