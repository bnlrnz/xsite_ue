<img src="/Doc/demo.jpg" width="400">
Example scene running an Unreal Engine Demo with the xsite_ue in our CAVE at the TU Bergakademie Freiberg computer science department.

# xsite_ue

Unreal Engine Cave Plugin for Unreal Engine Version 4.25.4 (4.25.0-4.25.4 or newer versions may or may not work...)
This Plugin is somewhat similar to nDisplay (but a lot smaller, easier). However, it runs on Linux, Mac and Windows.

Features:

- project plugin, no need to rebuild engine
- rendering a single scene with multiple or one computers
- each computer can render to several displays with different view frustums or just one
- there is only one instance per computer which will spawn the demanded windows
- off axis view frustums
- warping and blending support
- VRPN Support with head tracking, flystick navigation, interaction
- some templates for flystick interaction (draggable object, buttons)
- single configuration file (json) for all computers

## Demo Application
You can find a preconfigured demo project here: [xsite_ue_example](https://github.com/bnlrnz/xsite_ue_example)

## Installation
Clone into "Plugins" folder (if it is not there, create it in your project root) of your Unreal Engine Project or add as submodule. Initialize and pull git lfs.

```Bash
git clone git@github.com:bnlrnz/xsite_ue.git or git submodule add git@github.com:bnlrnz/xsite_ue.git
cd xsite_ue
git lfs install
git lfs pull
```

## Building
The Editor should build the Plugin on startup. You can build it yourself from the editor (if you change something in the plugins code) by enabling the "Modules" Panel (Window -> Developer Tools -> Modules). In the "Modules" Panel you can search for "xsite_ue" and press recompile. This will also trigger Hot Reload for the plugin.

## Show Plugin Content
To display the plugins content (sources, materials, blueprints ...) in the "Content Browser", click on "View Options" (eye symbol in the bottom right of the "Content Browser") and activate "Show Plugin Content".

![](/Doc/set3.png)

## Setting Up the Plugin
- Enable the Plugin in the Project Settings. (It should be under "Project" not "Built-In"; Scroll all the way down in the plugins windows left bar.)
- Select "OffAxisLocalPlayer" as Local Player Class in the Engine - General Settings

![](/Doc/set1.png)

- Select "CaveGameInstance" as Game Instance in the Project - Maps & Modes

![](/Doc/GameInstance.png)

- Select "CaveGameModeBase" as Default GameMode in the Project - Maps & Modes

![](/Doc/set2.png)

- Import Input Settings from [InputBackup.ini](https://github.com/bnlrnz/xsite_ue/blob/main/InputBackup.ini)

![](Doc/input_setup.png)

## Getting started

Add a **CaveControllerActor Blueprint** from the xsite_ue Content folder to the scene (the location is not important). Select the **CaveControllerActor** in the "World Outliner". Down in the "Details" Panel you will find a few things to set up the Controller:

- Configuration File Path: this should point to a JSON file with all the relevant information for distributed rendering, warping, blending...
- BlendTexture Folder Path: this should point to a folder where all the blend/alpha textures are stored

Since multiple instances of Unreal Projects will share the Configuration and the Blend/Alpha textures, they should be situated somewhere outside the projects folders.

Disable **Temporal Anti-Aliasing** (you can choose "None" or any other AA method) and **Motion Blur** in the project settings. These effect don't work properly!

### Tips

You can execute some predefined console commands for the cave environment. [TAB] opens the console. Type Cave_ prefix to see commands.

Cave_Execute "..." executes commands on the server and all clients. This is useful for debugging or performance tweaking:

- Cave_Execute Gamma 0-10 (if screens are too dark)
- Cave_Execute r.ScreenPercentage 0-100 (Scale render resolution, Performance)
- Cave_Execute sg.PostProcessQuality 0-3 (Performance)
- Cave_Execute r.PostProcessAAQuality 0-6 (Performace, depending on you AA method)
- See [ScalabilityReference](https://docs.unrealengine.com/en-US/Engine/Performance/Scalability/ScalabilityReference/index.html) for more Performance Options/Tweaks

