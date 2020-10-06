# xsite_ue

Unreal Engine Cave Plugin for Version 4.25. This works on Linux only for now because the plugin bundles VRPN   

## Installation
Clone into Plugin-Folder of your Unreal Engine (V4.25) Project. Initialize and pull git lfs.

```Bash
git clone git@github.com:bnlrnz/xsite_ue.git
cd uecave
git lfs install
git lfs pull
```

## Building
The Editor should build the Plugin on startup. You can build it yourself from the editor (if you change something in the plugins code) by enabling the Modules Panel (Window -> Developer Tools -> Modules). In the Modules Panel you can search for "xsite_ue" and press recompile. This will also trigger Hot Reload for the plugin.

## Plugin Content
To display the plugins content (sources, materials, blueprints ...) in the Content Browser, click on "View Options" (eye symbol in the bottom right of the Content Browser) and activate "Show Plugin Content".

![](/Doc/set3.png)

## Setting Up the Plugin
- Enable the Plugin in the Project Settings.
- Select "OffAxisLocalPlayer" as Local Player Class in the Engine - General Settings

![](/Doc/set1.png)

- Select "CaveGameModeBase" as Default GameMode in the Project - Maps & Modes

![](/Doc/set2.png)

