## Building Source Level Editor (SLE) from Source Code

### Disclaimer

SLE is based on Valve's Hammer editor circa 2013. There's no publically available version of it to fork off of, so this project is built to be entirely fan-oriented, non-commercial, and does not claim or imply any endorsement or support from Valve. The entire reason for this repository, and the source code release, is to give back to the modding community and try to build something cool.

Part of the additional source code comes from the outdated (and since then closed) repository of Momentum Mod.

All rights for Hammer belong to Valve. 

### Requirements

The solution file can be found at src\source_level_editor.sln.

The project is currently made to be built on Visual Studio 2015, with v120_xp build tools. It stems from my personal developer setup and preferences.
Being a solo dev, I cannot manage porting it to newer versions or making it multi-platform at this time.
Upon opening the solution for the first time, you will be prompted about upgrading it. I do not, for the sake of keeping it compatible with the same tools the Source 2013 SDK is being built with, and for the sake of supporting Win XP. 
Your experience using updated tools may vary.

### Special notes

The project comes with Release and Release-Manifest configurations. The only point of difference is Release-Manifest enables the Windows Manifest which makes the window and the menus look more 'modern'.

The built .exe and .dll files will be placed in src\_compiled_bin. The source code is provided with pre-built fmod.dll, already placed inside of src\_compiled_bin. You will it alongside level_editor.exe in order to run it (see the installation instructions in README.md).

### Known issues

The Debug configuration is currently non-compilable. Choose Release or Release-Manifest.
