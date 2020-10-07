# xsite_ue

Unreal Engine Cave Plugin for Version 4.25.

## Demo Application
You can find a preconfigured demo project here: [xsite_ue_example](https://github.com/bnlrnz/xsite_ue_example)

## Dependencies
This works on Linux only for now because the plugin bundles [VRPN](https://github.com/vrpn/vrpn) precompiled for Linux. This should be easily replaceable with the Windows version. Make shure to compile VRPN with the same compiler as Unreal Engine. 

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

## Getting started

Add a **CaveControllerActor Blueprint** from the xsite_ue Content folder to the scene (the location is not important). Select the **CaveControllerActor** in the "World Outliner". Down in the "Details" Panel you will find a few things to set up the Controller:

- Configuration File Path: this should point to a JSON file with all the relevant information for distributed rendering, warping, blending...
- BlendTexture Folder Path: this should point to a folder where all the blend/alpha textures are stored

Since multiple instances of Unreal Projects will share the Configuration and the Blend/Alpha textures, they should be situated somewhere outside the projects folders.


