# MinecraftPS3 PC Port

Windows x64 port of Minecraft Legacy Console Edition based on the sources archived by [M2noa/MCLCE-Archive](https://github.com/M2noa/MCLCE-Archive/). Credit to the original developers and archive maintainers.

## Playing on Windows

Download the Windows x64 ZIP from the releases page, extract all files and run `Minecraft.Client.exe`. The port supports keyboard and mouse, XInput controllers, and supported PlayStation controllers via the included SDL3 runtime. Original DualShock 3 controllers may require a compatible Windows driver. On hybrid systems the game prefers a high-performance GPU and falls back to another available adapter.

Some graphics settings require a restart. Worlds are currently read and saved through `Windows64/GameHDD` in the game directory; moving them to `Documents/MinecraftPS3/Worlds` is not complete. Graphics settings are stored in `Documents/MinecraftPS3/pc_graphics.ini` when available. Controller behaviour on all physical devices has not been verified.

## Building

Open `MinecraftConsoles.sln` in Visual Studio with the Visual C++ 2012 (v110) toolset and select `Release | Windows64`. The resulting executable is written to `x64/Release`. Runtime resources are loaded from `Minecraft.Client/Common/Media/MediaWindows64.arc`.

## Source attribution

Source archive: [M2noa/MCLCE-Archive](https://github.com/M2noa/MCLCE-Archive/). Minecraft and its assets belong to their respective owners. No license is implied for the original game or third-party components. SDL3 is distributed with its accompanying license in `Minecraft.Client/Windows64/SDL3/LICENSE.txt`.
