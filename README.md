# MinecraftPS3 PC Port

Experimental Windows x64 port of Minecraft Legacy Console Edition based on the sources archived by [M2noa/MCLCE-Archive](https://github.com/M2noa/MCLCE-Archive/). Credit to the original developers and archive maintainers.

## Windows build

Open `MinecraftConsoles.sln` in Visual Studio with the Visual C++ 2012 (v110) toolset and select `Release | Windows64`. The resulting executable is written to `x64/Release`. Runtime resources are loaded from `Minecraft.Client/Common/Media/MediaWindows64.arc`.

This is an experimental port. Some graphics settings require a restart. The move of worlds to Documents/MinecraftPS3/Worlds is not complete: existing worlds are still read and saved through Minecraft.Client/Windows64/GameHDD. Graphics settings are stored in Documents/MinecraftPS3/pc_graphics.ini when available.

## Source attribution

Source archive: [M2noa/MCLCE-Archive](https://github.com/M2noa/MCLCE-Archive/). Minecraft and its assets belong to their respective owners. No license is implied for the original game or third-party components.
