## Source Level Editor (SLE)

This is the repository of "SLE" - the Source Level Editor, my custom fork of Hammer with numerous fixes and new features.

![sle_splash](https://github.com/user-attachments/assets/30cc2ee3-15da-401b-9f0d-9226e0e09567)

The current version is 2.104.

In the Releases you'll find two versions: the regular and "nomanifest". You will most likely want the regular one. "Nomanifest" means its menus, buttons and dialogues follow the classic Win 9x visual theme, without smooth corners or Aero effects.

### 0. Nature and state of the project

It's a fork of Valve's Hammer editor, made from Source 2013 Hammer code. Its current version is 2.104. This is considered a beta release. It is not fully stable and hasn't been fully tested on the most recent operating systems.

Being contained in its own executable/folder, it could be installed and used in parallel with stock Hammer as well as other forks (although it hasn't been thoroughly tested), but watch out when saving your VMF projects across different editors, as they may strip some of the data if they're not familiar with it (this is the unavoidable result of how VMFs are saved in general). 

### 1. Installation

Unpack the downloaded release archive into the \bin\ folder of your target base game; in other words, the same location as Hammer 4.1. Depending on your base game, it will likely be [Steam directory]\Steamapps\common\Source SDK Base 2013 [Singleplayer or Multiplayer]\bin\ or [Steam directory]\Steamapps\common\Half-Life 2\bin\.

Launch level_editor.exe and follow the Setup Guide provided in the level_editor.chm file.

### 2. Documentation

SLE comes with offline help file, level_editor.chm. It is build similar to Worldcraft 3.x help file.
Using the Help menu in the editor opens that file; it can also be opened in Windows directly.
It contains the Setup Guide and User's Guide on most of new features and basic options and toolbars overview.

The online version will be hosted on SLE's repo's internal wiki.

### 3. Supported games

As the name suggests, it's the core Source 1 games: Half-Life 2, Portal, CS:S, TF2, Source SDK 2013. More modern games that use more modern Source - CS:GO, L4D, Portal 2, Alien Swarm, and Garry's Mod - aren't supported or weren't tested. 

### 4. Notable features

* Face Edit tool supports texture shearing, randomised X/Y shifting, user-defined shifting step, extra Fit modes, quick Nodraw and Mark buttons, Grow/Shrink selection buttons;

* Displacement tool supports 45° X/Y axii sculpting;

* Multiple Cordons are supported;

* Material Browser displays transparency, detail type, material shader, reflectivity; counts decals among used materials; the texture cache size can be set in the Options->Materials tab;

* 3D Viewport can display the skybox, fog; can display scaled 3D skybox by marking 3D skybox geometry for display (no instancing required);

* Ropes, Beams and Spotlights are rendered in 3D;

* Worldtext helper is supported, as is point_message preview;

* Soundscapes can be parsed and selected in the Sound Browser;

* Actbusy script is parsed for the drop-down menu for hint nodes;

* Users can hide Tool brushes in 3D view and hide Editor objects (info/env/point/logic entities) (this will only hide them from view);

* Model entities' scale and rendercolor are previewed in 3D;

* Viewports support custom colours;

* Transformation tool can remember settings and use Local Transform;

* Vertex Scale dialogue has been reworked to work same as Transformation tool;

* Rotating objects now supports independent Rotation Pivot;

* Rotating objects while holding Shift now clones them (with rotation applied);

* Vertex precision for solids is improved, in particular for Cylinders and Spheres;

* Sound Browser has FMod support allowing for MP3 playback, volume slider, and Stop button;

* Placing entities has Align to Surface checkbox for working in 3D;

* Block Tool now supports creating brushes in 3D viewport;

* Switching Select Modes (Groups-Objects-Solids) now keeps the selection instead of clearing it;

* Flat View previews complexity of models (tints them according to their triangle count);

* Overlays keep/update face assignment when using Clipper Tool on their solids;

* Limited support for HL1 mapping: can save brushwork, entities and texture coordinates (but cannot load from WADs or use non-power of 2 textures);

The entire list of changes and features can be found at:
https://docs.google.com/spreadsheets/d/1uqCHjbjhXvRDr77jTaQFajo1EvFPy297ZsINmf5sF90

### 5. Development overview

I started working on it several years ago (in November 2020) when I began feeling like vanilla Hammer's usability was getting constrictive. Over these several years I've tweaked and expanded Hammer first and foremost to suit my own mapping needs, as I spend a lot of time making maps. Among the first features were not only fixes for common bugs, but efficiency shortcuts and simple automation. As time went on, the changes amassed to the point where there were hundreds of them, and this started to feel like its own tool.

### 6. "Why release now? Who is it for?"

Variety is the spice of life, as the saying goes. Granted, I realise I'm *years* too late to the cause, with the oft-prophetised death of Source modding now closer than ever, what's with people retiring, forums almost having fallen out of use, and modding as a whole having taken a different direction and face than during the golden years. I also realise most people already chose something else, and don't need this.

Nevertheless, it felt more wrong to just let this project fade away without anyone knowing. 

So this is being released (with updates planned regardless of attention) to anyone who may want to try it. Anyone who wants this as an option. Anyone who may find some of its features novel and appealing enough. Maybe no one will, maybe no one needs it. However likely that is, this tool is here now.

### 7. "Why isn't feature X present?"

The likely answer is either I'd like to add it but it'll take more time, or it's something I don't find useful so it doesn't have a high priority or may not have been considered. Nevertheless, feel free to suggest new things, but keep in mind this project is still working within the confines of MFC and ~WinXP era software sensibilities. And is made by a solo dev with limited experience.

### 8. Open source

In the future (within this year, hopefully) this tool should get open-sourced. There's some clean-up and documentation needed to be done, as well as finalising some feature plans. This is another reason for releasing this in the first place: to also make it open-source unlike other known Hammer forks.

### 9. Credits

Valve Hammer Editor belongs to Valve. This project is not in any way affiliated or endorsed by Valve, nor is it based on officially available Valve code.

This project is developed and maintained by Cvoxalury.

Thanks to now-defunct SDK-2013-Hammer project for sharing their source code; some of the features seen here, such as material proxies, material browser transparency preview, FMod support, custom colours support, and multiple cordons upport, wouldn't be done without it.
