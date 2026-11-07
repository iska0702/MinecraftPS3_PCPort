// Minecraft.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <Xinput.h>
#include <ShlObj.h>
#include <dxgi1_2.h>
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "dxgi.lib")

// These exports ask hybrid-graphics drivers to prefer the discrete GPU.
extern "C" {
	__declspec(dllexport) DWORD NvOptimusEnablement = 1;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

#include <assert.h>
#include "GameConfig\Minecraft.spa.h"
#include "..\MinecraftServer.h"
#include "..\LocalPlayer.h"
#include "..\..\Minecraft.World\ItemInstance.h"
#include "..\..\Minecraft.World\MapItem.h"
#include "..\..\Minecraft.World\Recipes.h"
#include "..\..\Minecraft.World\Recipy.h"
#include "..\..\Minecraft.World\Language.h"
#include "..\..\Minecraft.World\StringHelpers.h"
#include "..\..\Minecraft.World\AABB.h"
#include "..\..\Minecraft.World\Vec3.h"
#include "..\..\Minecraft.World\Level.h"
#include "..\..\Minecraft.World\net.minecraft.world.level.tile.h"

#include "..\ClientConnection.h"
#include "..\User.h"
#include "..\..\Minecraft.World\Socket.h"
#include "..\..\Minecraft.World\ThreadName.h"
#include "..\..\Minecraft.Client\StatsCounter.h"
#include "..\ConnectScreen.h"
//#include "Social\SocialManager.h"
//#include "Leaderboards\LeaderboardManager.h"
//#include "XUI\XUI_Scene_Container.h"
//#include "NetworkManager.h"
#include "..\..\Minecraft.Client\Tesselator.h"
#include "..\..\Minecraft.Client\Options.h"
#include "..\LevelRenderer.h"
#include "Sentient\SentientManager.h"
#include "..\..\Minecraft.World\IntCache.h"
#include "..\Textures.h"
#include "Resource.h"
#include "..\..\Minecraft.World\compression.h"
#include "..\..\Minecraft.World\OldChunkStorage.h"

#include "Xbox/resource.h"

HINSTANCE hMyInst;
LRESULT CALLBACK DlgProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
char chGlobalText[256];
uint16_t ui16GlobalText[256];

#define THEME_NAME		"584111F70AAAAAAA"
#define THEME_FILESIZE	2797568

//#define THREE_MB 3145728 // minimum save size (checking for this on a selected device)
//#define FIVE_MB 5242880 // minimum save size (checking for this on a selected device)
//#define FIFTY_TWO_MB (1024*1024*52) // Maximum TCR space required for a save (checking for this on a selected device)
#define FIFTY_ONE_MB (1000000*51) // Maximum TCR space required for a save is 52MB (checking for this on a selected device)

//#define PROFILE_VERSION 3 // new version for the interim bug fix 166 TU
#define NUM_PROFILE_VALUES	5
#define NUM_PROFILE_SETTINGS 4
DWORD dwProfileSettingsA[NUM_PROFILE_VALUES]=
{
#ifdef _XBOX
	XPROFILE_OPTION_CONTROLLER_VIBRATION,
	XPROFILE_GAMER_YAXIS_INVERSION,
	XPROFILE_GAMER_CONTROL_SENSITIVITY,
	XPROFILE_GAMER_ACTION_MOVEMENT_CONTROL,
	XPROFILE_TITLE_SPECIFIC1,
#else
	0,0,0,0,0
#endif
};

//-------------------------------------------------------------------------------------
// Time             Since fAppTime is a float, we need to keep the quadword app time
//                  as a LARGE_INTEGER so that we don't lose precision after running
//                  for a long time.
//-------------------------------------------------------------------------------------

BOOL g_bWidescreen = TRUE;

int g_iScreenWidth = 1920;
int g_iScreenHeight = 1080;

void DefineActions(void)
{
	// The app needs to define the actions required, and the possible mappings for these

	// Split into Menu actions, and in-game actions

	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_A,							_360_JOY_BUTTON_A);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_B,							_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_X,							_360_JOY_BUTTON_X);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_Y,							_360_JOY_BUTTON_Y);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OK,							_360_JOY_BUTTON_A);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_CANCEL,						_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_UP,							_360_JOY_BUTTON_DPAD_UP | _360_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_DOWN,						_360_JOY_BUTTON_DPAD_DOWN | _360_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_LEFT,						_360_JOY_BUTTON_DPAD_LEFT | _360_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_RIGHT,						_360_JOY_BUTTON_DPAD_RIGHT | _360_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_PAGEUP,						_360_JOY_BUTTON_LT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_PAGEDOWN,					_360_JOY_BUTTON_RT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_RIGHT_SCROLL,				_360_JOY_BUTTON_RB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_LEFT_SCROLL,					_360_JOY_BUTTON_LB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_PAUSEMENU,					_360_JOY_BUTTON_START);

	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_STICK_PRESS,					_360_JOY_BUTTON_LTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OTHER_STICK_PRESS,			_360_JOY_BUTTON_RTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OTHER_STICK_UP,				_360_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OTHER_STICK_DOWN,			_360_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OTHER_STICK_LEFT,			_360_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,ACTION_MENU_OTHER_STICK_RIGHT,			_360_JOY_BUTTON_RSTICK_RIGHT);

	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_JUMP,					_360_JOY_BUTTON_A);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_FORWARD,				_360_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_BACKWARD,				_360_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LEFT,					_360_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_RIGHT,					_360_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LOOK_LEFT,				_360_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LOOK_RIGHT,				_360_JOY_BUTTON_RSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LOOK_UP,				_360_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LOOK_DOWN,				_360_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_USE,					_360_JOY_BUTTON_LT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_ACTION,					_360_JOY_BUTTON_RT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_RIGHT_SCROLL,			_360_JOY_BUTTON_RB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_LEFT_SCROLL,			_360_JOY_BUTTON_LB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_INVENTORY,				_360_JOY_BUTTON_Y);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_PAUSEMENU,				_360_JOY_BUTTON_START);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_DROP,					_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_SNEAK_TOGGLE,			_360_JOY_BUTTON_RTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_CRAFTING,				_360_JOY_BUTTON_X);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_RENDER_THIRD_PERSON,	_360_JOY_BUTTON_LTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_GAME_INFO,				_360_JOY_BUTTON_BACK);

	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_DPAD_LEFT,				_360_JOY_BUTTON_DPAD_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_DPAD_RIGHT,				_360_JOY_BUTTON_DPAD_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_DPAD_UP,				_360_JOY_BUTTON_DPAD_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_0,MINECRAFT_ACTION_DPAD_DOWN,				_360_JOY_BUTTON_DPAD_DOWN);

	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_A,							_360_JOY_BUTTON_A);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_B,							_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_X,							_360_JOY_BUTTON_X);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_Y,							_360_JOY_BUTTON_Y);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_OK,							_360_JOY_BUTTON_A);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_CANCEL,						_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_UP,							_360_JOY_BUTTON_DPAD_UP | _360_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_DOWN,						_360_JOY_BUTTON_DPAD_DOWN | _360_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_LEFT,						_360_JOY_BUTTON_DPAD_LEFT | _360_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_RIGHT,						_360_JOY_BUTTON_DPAD_RIGHT | _360_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_PAGEUP,						_360_JOY_BUTTON_LB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_PAGEDOWN,					_360_JOY_BUTTON_RT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_RIGHT_SCROLL,				_360_JOY_BUTTON_RB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_LEFT_SCROLL,					_360_JOY_BUTTON_LB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_PAUSEMENU,					_360_JOY_BUTTON_START);

	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_STICK_PRESS,					_360_JOY_BUTTON_LTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_OTHER_STICK_PRESS,			_360_JOY_BUTTON_RTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_OTHER_STICK_UP,				_360_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_OTHER_STICK_DOWN,			_360_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_OTHER_STICK_LEFT,			_360_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,ACTION_MENU_OTHER_STICK_RIGHT,			_360_JOY_BUTTON_RSTICK_RIGHT);

	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_JUMP,					_360_JOY_BUTTON_RB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_FORWARD,				_360_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_BACKWARD,				_360_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LEFT,					_360_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_RIGHT,					_360_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LOOK_LEFT,				_360_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LOOK_RIGHT,				_360_JOY_BUTTON_RSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LOOK_UP,				_360_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LOOK_DOWN,				_360_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_USE,					_360_JOY_BUTTON_RT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_ACTION,					_360_JOY_BUTTON_LT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_RIGHT_SCROLL,			_360_JOY_BUTTON_DPAD_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_LEFT_SCROLL,			_360_JOY_BUTTON_DPAD_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_INVENTORY,				_360_JOY_BUTTON_Y);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_PAUSEMENU,				_360_JOY_BUTTON_START);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_DROP,					_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_SNEAK_TOGGLE,			_360_JOY_BUTTON_LTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_CRAFTING,				_360_JOY_BUTTON_X);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_RENDER_THIRD_PERSON,	_360_JOY_BUTTON_RTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_GAME_INFO,				_360_JOY_BUTTON_BACK);
	
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_DPAD_LEFT,				_360_JOY_BUTTON_DPAD_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_DPAD_RIGHT,				_360_JOY_BUTTON_DPAD_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_DPAD_UP,				_360_JOY_BUTTON_DPAD_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_1,MINECRAFT_ACTION_DPAD_DOWN,				_360_JOY_BUTTON_DPAD_DOWN);	

	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_A,							_360_JOY_BUTTON_A);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_B,							_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_X,							_360_JOY_BUTTON_X);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_Y,							_360_JOY_BUTTON_Y);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OK,							_360_JOY_BUTTON_A);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_CANCEL,						_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_UP,							_360_JOY_BUTTON_DPAD_UP | _360_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_DOWN,						_360_JOY_BUTTON_DPAD_DOWN | _360_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_LEFT,						_360_JOY_BUTTON_DPAD_LEFT | _360_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_RIGHT,						_360_JOY_BUTTON_DPAD_RIGHT | _360_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_PAGEUP,						_360_JOY_BUTTON_DPAD_UP | _360_JOY_BUTTON_LB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_PAGEDOWN,					_360_JOY_BUTTON_RT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_RIGHT_SCROLL,				_360_JOY_BUTTON_RB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_LEFT_SCROLL,					_360_JOY_BUTTON_LB);

	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_JUMP,					_360_JOY_BUTTON_LT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_FORWARD,				_360_JOY_BUTTON_LSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_BACKWARD,				_360_JOY_BUTTON_LSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LEFT,					_360_JOY_BUTTON_LSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_RIGHT,					_360_JOY_BUTTON_LSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LOOK_LEFT,				_360_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LOOK_RIGHT,				_360_JOY_BUTTON_RSTICK_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LOOK_UP,				_360_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LOOK_DOWN,				_360_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_USE,					_360_JOY_BUTTON_RT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_ACTION,					_360_JOY_BUTTON_A);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_RIGHT_SCROLL,			_360_JOY_BUTTON_DPAD_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_LEFT_SCROLL,			_360_JOY_BUTTON_DPAD_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_INVENTORY,				_360_JOY_BUTTON_Y);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_PAUSEMENU,				_360_JOY_BUTTON_START);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DROP,					_360_JOY_BUTTON_B);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_SNEAK_TOGGLE,			_360_JOY_BUTTON_LB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_CRAFTING,				_360_JOY_BUTTON_X);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_RENDER_THIRD_PERSON,	_360_JOY_BUTTON_LTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_GAME_INFO,				_360_JOY_BUTTON_BACK);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_PAUSEMENU,					_360_JOY_BUTTON_START);

	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_STICK_PRESS,					_360_JOY_BUTTON_LTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_PRESS,			_360_JOY_BUTTON_RTHUMB);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_UP,				_360_JOY_BUTTON_RSTICK_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_DOWN,			_360_JOY_BUTTON_RSTICK_DOWN);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_LEFT,			_360_JOY_BUTTON_RSTICK_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,ACTION_MENU_OTHER_STICK_RIGHT,			_360_JOY_BUTTON_RSTICK_RIGHT);
	
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DPAD_LEFT,				_360_JOY_BUTTON_DPAD_LEFT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DPAD_RIGHT,				_360_JOY_BUTTON_DPAD_RIGHT);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DPAD_UP,				_360_JOY_BUTTON_DPAD_UP);
	InputManager.SetGameJoypadMaps(MAP_STYLE_2,MINECRAFT_ACTION_DPAD_DOWN,				_360_JOY_BUTTON_DPAD_DOWN);	
}

#if 0
HRESULT InitD3D( IDirect3DDevice9 **ppDevice,
				D3DPRESENT_PARAMETERS *pd3dPP )
{
	IDirect3D9 *pD3D;

	pD3D = Direct3DCreate9( D3D_SDK_VERSION );

	// Set up the structure used to create the D3DDevice
	// Using a permanent 1280x720 backbuffer now no matter what the actual video resolution.right Have also disabled letterboxing,
	// which would letterbox a 1280x720 output if it detected a 4:3 video source - we're doing an anamorphic squash in this
	// mode so don't need this functionality.

	ZeroMemory( pd3dPP, sizeof(D3DPRESENT_PARAMETERS) );
	XVIDEO_MODE VideoMode;
	XGetVideoMode( &VideoMode );
	g_bWidescreen = VideoMode.fIsWideScreen;
	pd3dPP->BackBufferWidth        = 1280;
	pd3dPP->BackBufferHeight       = 720;
	pd3dPP->BackBufferFormat       = D3DFMT_A8R8G8B8;
	pd3dPP->BackBufferCount        = 1;
	pd3dPP->EnableAutoDepthStencil = TRUE;
	pd3dPP->AutoDepthStencilFormat = D3DFMT_D24S8;
	pd3dPP->SwapEffect             = D3DSWAPEFFECT_DISCARD;
	pd3dPP->PresentationInterval   = D3DPRESENT_INTERVAL_ONE;
	//pd3dPP->Flags				   = D3DPRESENTFLAG_NO_LETTERBOX;
	//ERR[D3D]: Can't set D3DPRESENTFLAG_NO_LETTERBOX when wide-screen is enabled
	//	in the launcher/dashboard.
	if(g_bWidescreen)
		pd3dPP->Flags=0;
	else
		pd3dPP->Flags				   = D3DPRESENTFLAG_NO_LETTERBOX;

	// Create the device.
	return pD3D->CreateDevice(
		0,
		D3DDEVTYPE_HAL,
		NULL,
		D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_BUFFER_2_FRAMES,
		pd3dPP,
		ppDevice );
}
#endif
//#define MEMORY_TRACKING

#ifdef MEMORY_TRACKING
void ResetMem();
void DumpMem();
void MemPixStuff();
#else
void MemSect(int sect)
{
}
#endif

HINSTANCE               g_hInst = NULL;
HWND                    g_hWnd = NULL;
// Windows display choices are read at startup because the renderer owns fixed-size targets.
int g_pcDisplayMode = 0; // 0 windowed, 1 borderless, 2 exclusive fullscreen
int g_pcOutputWidth = 1920, g_pcOutputHeight = 1080, g_pcRenderScale = 100;
int g_pcGraphicsQuality = 2;
int g_pcViewDistance = 0;
DWORD g_pcEscapeBlockedUntil = 0;
int g_pcMouseSensitivity = 100, g_pcMouseInvert = 0;
int g_pcAspectRatio = 0; // Auto, 4:3, 5:4, 3:2, 16:10, 16:9, 21:9, 32:9, 32:10, 1:1, 3:4, 9:16
HWND g_pcBackdrop = NULL;
const int g_pcAspectNumerators[] = {0,4,5,3,16,16,64,32,16,1,3,9,2,20,27};
const int g_pcAspectDenominators[] = {0,3,4,2,10,9,27,9,5,1,4,16,1,9,64};
int g_pcAspectCount = sizeof(g_pcAspectNumerators) / sizeof(g_pcAspectNumerators[0]);

void PCGetCanvasSize(int &width, int &height)
{
	width = g_pcDisplayMode == 1 ? GetSystemMetrics(SM_CXSCREEN) : g_pcOutputWidth;
	height = g_pcDisplayMode == 1 ? GetSystemMetrics(SM_CYSCREEN) : g_pcOutputHeight;
	if (g_pcAspectRatio > 0 && g_pcAspectRatio < g_pcAspectCount)
	{
		int n = g_pcAspectNumerators[g_pcAspectRatio], d = g_pcAspectDenominators[g_pcAspectRatio];
		if ((__int64)width * d > (__int64)height * n) width = height * n / d;
		else height = width * d / n;
	}
}
bool g_pcCursorHidden = false;
bool g_pcUsingGamepad = false;
volatile LONG g_pcRawMouseX = 0, g_pcRawMouseY = 0;
bool g_pcRawMouseRegistered = false;
bool g_pcPlayStationConnected = false;
namespace PCSonyPad { static bool IsAvailable(); }
int g_pcFov = 70;
unsigned int g_pcInputGeneration = 0;
DWORD g_pcLastKeyboardMouseInput = 0;

void PCSetInputDevice(bool gamepad)
{
	if (!gamepad) g_pcLastKeyboardMouseInput = GetTickCount();
	if (gamepad != g_pcUsingGamepad) { g_pcUsingGamepad = gamepad; ++g_pcInputGeneration; }
}

// Detect physical Sony pads separately from XInput. A compatibility driver can
// expose the same pad as an Xbox device, but the controls artwork should still
// use the PlayStation 3 layout when the original Sony device is visible.
void PCDetectPlayStationController()
{
	UINT count = 0;
	if (GetRawInputDeviceList(NULL, &count, sizeof(RAWINPUTDEVICELIST)) != 0) return;
	std::vector<RAWINPUTDEVICELIST> devices(count);
	if (count && GetRawInputDeviceList(&devices[0], &count, sizeof(RAWINPUTDEVICELIST)) == (UINT)-1) return;
	bool found = PCSonyPad::IsAvailable();
	for (UINT i = 0; !found && i < count; ++i)
	{
		if (devices[i].dwType != RIM_TYPEHID) continue;
		RID_DEVICE_INFO info = {0};
		info.cbSize = sizeof(info);
		UINT size = sizeof(info);
		if (GetRawInputDeviceInfoW(devices[i].hDevice, RIDI_DEVICEINFO, &info, &size) == (UINT)-1) continue;
		if (info.hid.dwVendorId != 0x054c || info.hid.usUsagePage != 1 ||
			(info.hid.usUsage != 4 && info.hid.usUsage != 5)) continue;
		switch (info.hid.dwProductId)
		{
		case 0x0268: // DualShock 3 / Sixaxis
		case 0x05c4: case 0x09cc: // DualShock 4
		case 0x0ce6: case 0x0df2: // DualSense and DualSense Edge
			found = true;
			break;
		}
		if (found) break;
	}
	if (found != g_pcPlayStationConnected)
	{
		g_pcPlayStationConnected = found;
		++g_pcInputGeneration;
	}
}

#include "PCGamepadSDL.inl"

void PCPollGamepad()
{
	static DWORD lastSonyCheck = 0;
	DWORD now = GetTickCount();
	if (!lastSonyCheck || now - lastSonyCheck >= 1000)
	{
		PCDetectPlayStationController();
		lastSonyCheck = now;
	}
	static XINPUT_GAMEPAD last = {0};
	static bool wasConnected = false;
	XINPUT_STATE state = {0};
	bool connected = XInputGetState(0, &state) == ERROR_SUCCESS;
	if (!connected)
	{
		if (wasConnected && g_pcUsingGamepad) PCSetInputDevice(false);
		ZeroMemory(&last, sizeof(last)); wasConnected = false; return;
	}
	XINPUT_GAMEPAD current = state.Gamepad;
	if (abs(current.sThumbLX) < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) current.sThumbLX = 0;
	if (abs(current.sThumbLY) < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) current.sThumbLY = 0;
	if (abs(current.sThumbRX) < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) current.sThumbRX = 0;
	if (abs(current.sThumbRY) < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) current.sThumbRY = 0;
	if (current.bLeftTrigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD) current.bLeftTrigger = 0;
	if (current.bRightTrigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD) current.bRightTrigger = 0;
	// Ignore small stick/trigger fluctuations after a keyboard or mouse event.
	// A new button or trigger press still switches immediately to the gamepad.
	bool buttonPressed = (current.wButtons & ~last.wButtons) != 0;
	bool triggerPressed = (current.bLeftTrigger >= 128 && last.bLeftTrigger < 128) ||
		(current.bRightTrigger >= 128 && last.bRightTrigger < 128);
	const int switchDeadzone = 18000;
	bool stickMoved = (abs(current.sThumbLX) > switchDeadzone && abs(last.sThumbLX) <= switchDeadzone) ||
		(abs(current.sThumbLY) > switchDeadzone && abs(last.sThumbLY) <= switchDeadzone) ||
		(abs(current.sThumbRX) > switchDeadzone && abs(last.sThumbRX) <= switchDeadzone) ||
		(abs(current.sThumbRY) > switchDeadzone && abs(last.sThumbRY) <= switchDeadzone);
	bool mouseSettled = !g_pcLastKeyboardMouseInput || now - g_pcLastKeyboardMouseInput >= 350;
	if (GetForegroundWindow() == g_hWnd && (buttonPressed || triggerPressed || (mouseSettled && stickMoved))) PCSetInputDevice(true);
	last = current; wasConnected = true;
}
wchar_t g_pcGraphicsPath[MAX_PATH] = {0};

void PCUpdateCursor(bool hide)
{
	if (hide == g_pcCursorHidden) return;
	if (hide) { while (ShowCursor(FALSE) >= 0) {} }
	else { while (ShowCursor(TRUE) < 0) {} }
	g_pcCursorHidden = hide;
}

void PCLoadGraphicsSettings()
{
	wchar_t oldPath[MAX_PATH] = {0};
	GetModuleFileNameW(NULL, oldPath, MAX_PATH);
	wchar_t *slash = wcsrchr(oldPath, L'\\');
	if (slash) *(slash + 1) = 0; else oldPath[0] = 0;
	wcscat_s(oldPath, MAX_PATH, L"pc_graphics.ini");
	wchar_t documents[MAX_PATH] = {0};
	if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, documents)))
	{
		wchar_t dataDir[MAX_PATH] = {0};
		if (swprintf_s(dataDir, MAX_PATH, L"%s\\MinecraftPS3", documents) >= 0 &&
			(CreateDirectoryW(dataDir, NULL) || GetLastError() == ERROR_ALREADY_EXISTS) &&
			swprintf_s(g_pcGraphicsPath, MAX_PATH, L"%s\\pc_graphics.ini", dataDir) >= 0)
		{
			// Preserve existing settings without replacing a newer file.
			CopyFileW(oldPath, g_pcGraphicsPath, TRUE);
		}
	}
	if (!g_pcGraphicsPath[0]) wcscpy_s(g_pcGraphicsPath, MAX_PATH, oldPath);
	g_pcDisplayMode = GetPrivateProfileIntW(L"Graphics", L"Mode", 0, g_pcGraphicsPath);
	g_pcOutputWidth = GetPrivateProfileIntW(L"Graphics", L"Width", 1920, g_pcGraphicsPath);
	g_pcOutputHeight = GetPrivateProfileIntW(L"Graphics", L"Height", 1080, g_pcGraphicsPath);
	g_pcRenderScale = GetPrivateProfileIntW(L"Graphics", L"RenderScale", 100, g_pcGraphicsPath);
	g_pcFov = GetPrivateProfileIntW(L"Graphics", L"FOV", 70, g_pcGraphicsPath);
	if (g_pcFov < 60 || g_pcFov > 110 || g_pcFov % 5 != 0) g_pcFov = 70;
	g_pcGraphicsQuality = GetPrivateProfileIntW(L"Graphics", L"Quality", 2, g_pcGraphicsPath);
	g_pcViewDistance = GetPrivateProfileIntW(L"Graphics", L"ViewDistance", 0, g_pcGraphicsPath);
	if (g_pcViewDistance < 0 || g_pcViewDistance > 3) g_pcViewDistance = 0;
	g_pcMouseSensitivity = GetPrivateProfileIntW(L"Graphics", L"MouseSensitivity", 100, g_pcGraphicsPath);
	g_pcMouseInvert = GetPrivateProfileIntW(L"Graphics", L"MouseInvert", 0, g_pcGraphicsPath) ? 1 : 0;
	if (g_pcMouseSensitivity < 0 || g_pcMouseSensitivity > 200) g_pcMouseSensitivity = 100;
	g_pcAspectRatio = GetPrivateProfileIntW(L"Graphics", L"AspectRatio", 0, g_pcGraphicsPath);
	// Preserve existing numeric IDs; retired ratios fall back to Auto.
	if (g_pcAspectRatio < 0 || g_pcAspectRatio >= g_pcAspectCount || g_pcAspectRatio == 2 || g_pcAspectRatio == 3) g_pcAspectRatio = 0;
	if (g_pcGraphicsQuality < 0 || g_pcGraphicsQuality > 2) g_pcGraphicsQuality = 2;
	if (g_pcDisplayMode < 0 || g_pcDisplayMode > 2) g_pcDisplayMode = 0;
	if (g_pcOutputWidth < 640 || g_pcOutputWidth > 7680 || g_pcOutputHeight < 360 || g_pcOutputHeight > 4320) { g_pcOutputWidth = 1920; g_pcOutputHeight = 1080; }
	if (g_pcRenderScale != 50 && g_pcRenderScale != 75 && g_pcRenderScale != 100 && g_pcRenderScale != 125) g_pcRenderScale = 100;
	// Borderless fills the desktop, so its render buffer must follow the desktop aspect.
	int outputWidth, outputHeight;
	PCGetCanvasSize(outputWidth, outputHeight);
	g_iScreenWidth = outputWidth * g_pcRenderScale / 100;
	g_iScreenHeight = outputHeight * g_pcRenderScale / 100;
}

void PCSaveGraphicsInt(const wchar_t *key, int value)
{
	wchar_t text[32]; swprintf_s(text, 32, L"%d", value);
	WritePrivateProfileStringW(L"Graphics", key, text, g_pcGraphicsPath);
}

// Cycle through monitor modes and common window sizes, without duplicates.
void PCNextResolution()
{
	typedef std::pair<int, int> Resolution;
	std::vector<Resolution> modes;
	DEVMODEW mode = {0}; mode.dmSize = sizeof(mode);
	for (DWORD index = 0; EnumDisplaySettingsW(NULL, index, &mode); ++index)
	{
		if (mode.dmPelsWidth >= 640 && mode.dmPelsHeight >= 360 && mode.dmPelsWidth <= 7680 && mode.dmPelsHeight <= 4320 && mode.dmBitsPerPel >= 32)
			modes.push_back(Resolution(mode.dmPelsWidth, mode.dmPelsHeight));
	}
	if (g_pcDisplayMode != 2)
	{
		static const int common[][2] = {{800,600},{1024,768},{1280,720},{1280,800},{1280,1024},{1366,768},{1440,900},{1600,900},{1680,1050},{1920,1080},{1920,1200},{2560,1080},{2560,1440},{2560,1600},{3440,1440},{3840,2160},{5120,1440}};
		for (unsigned int i = 0; i < sizeof(common)/sizeof(common[0]); ++i)
			modes.push_back(Resolution(common[i][0], common[i][1]));
	}
	if (modes.empty()) return;
	std::sort(modes.begin(), modes.end());
	modes.erase(std::unique(modes.begin(), modes.end()), modes.end());
	Resolution selected(g_pcOutputWidth, g_pcOutputHeight);
	std::vector<Resolution>::iterator next = std::upper_bound(modes.begin(), modes.end(), selected);
	if (next == modes.end()) next = modes.begin();
	g_pcOutputWidth = next->first; g_pcOutputHeight = next->second;
}

// Presets control world detail independently of render scale and display size.
void PCApplyGraphicsQuality()
{
	Minecraft *game = Minecraft::GetInstance();
	if (!game || !game->options) return;
	game->options->fancyGraphics = g_pcGraphicsQuality == 2;
	game->options->ambientOcclusion = g_pcGraphicsQuality != 0;
	game->options->viewDistance = g_pcViewDistance;
	game->options->particles = 2 - g_pcGraphicsQuality;
	if (game->levelRenderer && game->level) game->levelRenderer->allChanged();
}

volatile LONG g_pcMouseWheel = 0;
volatile LONG g_pcMenuKeyPressed[256] = {0};
volatile LONG g_pcGameKeyPressed[256] = {0};
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pd3dDevice = NULL;
ID3D11DeviceContext*    g_pImmediateContext = NULL;
IDXGISwapChain*         g_pSwapChain = NULL;
ID3D11RenderTargetView* g_pRenderTargetView = NULL;
ID3D11DepthStencilView* g_pDepthStencilView = NULL;
ID3D11Texture2D*		g_pDepthStencilBuffer = NULL;

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_INPUT:
		{
			RAWINPUT raw;
			UINT size = sizeof(raw);
			UINT bytes = GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, &raw, &size,
				sizeof(RAWINPUTHEADER));
			if (bytes != (UINT)-1 && bytes >= sizeof(RAWINPUTHEADER) + sizeof(RAWMOUSE) &&
				raw.header.dwType == RIM_TYPEMOUSE &&
				!(raw.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) && GetForegroundWindow() == hWnd)
			{
				LONG dx = raw.data.mouse.lLastX, dy = raw.data.mouse.lLastY;
				if (dx || dy)
				{
					PCSetInputDevice(false);
					if (app.GetGameStarted() && !ui.GetMenuDisplayed(0))
					{
						InterlockedExchangeAdd(&g_pcRawMouseX, dx);
						InterlockedExchangeAdd(&g_pcRawMouseY, dy);
					}
				}
			}
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (GetForegroundWindow() == hWnd) PCSetInputDevice(false);
		if (wParam < 256 && !(lParam & (1LL << 30)))
		{
			InterlockedExchange(&g_pcMenuKeyPressed[wParam], 1);
			InterlockedExchange(&g_pcGameKeyPressed[wParam], 1);
		}
		break;
	case WM_MOUSEMOVE:
		{
			RECT r; GetClientRect(hWnd, &r);
			int x = (short)LOWORD(lParam), y = (short)HIWORD(lParam);
			if (GetForegroundWindow() == hWnd && (x != (r.left + r.right)/2 || y != (r.top + r.bottom)/2)) PCSetInputDevice(false);
		}
		break;
	case WM_LBUTTONDOWN:
		PCSetInputDevice(false);
		InterlockedExchange(&g_pcGameKeyPressed[VK_LBUTTON], 1);
		InterlockedExchange(&g_pcMenuKeyPressed[VK_LBUTTON], 1);
		break;
	case WM_RBUTTONDOWN:
		PCSetInputDevice(false);
		InterlockedExchange(&g_pcGameKeyPressed[VK_RBUTTON], 1);
		InterlockedExchange(&g_pcMenuKeyPressed[VK_RBUTTON], 1);
		break;
	case WM_MOUSEWHEEL:
		PCSetInputDevice(false);
		InterlockedExchangeAdd(&g_pcMouseWheel, (short)HIWORD(wParam));
		return 0;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		if (g_pcBackdrop) { DestroyWindow(g_pcBackdrop); g_pcBackdrop = NULL; }
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, "Minecraft");
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= "Minecraft";
	wcex.lpszClassName	= "MinecraftClass";
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	g_hInst = hInstance; // Store instance handle in our global variable

	int canvasWidth, canvasHeight;
	PCGetCanvasSize(canvasWidth, canvasHeight);
	RECT wr = {0, 0, canvasWidth, canvasHeight};
	DWORD windowStyle = g_pcDisplayMode == 1 ? WS_POPUP : (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX);
	int x = CW_USEDEFAULT, y = 0;
	if (g_pcDisplayMode == 1)
	{
		int screenWidth = GetSystemMetrics(SM_CXSCREEN), screenHeight = GetSystemMetrics(SM_CYSCREEN);
		x = (screenWidth - canvasWidth) / 2; y = (screenHeight - canvasHeight) / 2;
		if (canvasWidth != screenWidth || canvasHeight != screenHeight)
		{
			// A black owner window fills the unused area for a forced aspect ratio.
			g_pcBackdrop = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, L"STATIC", L"", WS_POPUP | SS_BLACKRECT,
				0, 0, screenWidth, screenHeight, NULL, NULL, hInstance, NULL);
			if (g_pcBackdrop) ShowWindow(g_pcBackdrop, SW_SHOWNOACTIVATE);
		}
	}
	else AdjustWindowRect(&wr, windowStyle, FALSE);
	g_hWnd = CreateWindow("MinecraftClass", "Minecraft", windowStyle, x, y,
		wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, hInstance, NULL);

	if (!g_hWnd)
	{
		return FALSE;
	}

	RAWINPUTDEVICE mouseDevice = {0};
	mouseDevice.usUsagePage = 1;
	mouseDevice.usUsage = 2;
	mouseDevice.hwndTarget = g_hWnd;
	g_pcRawMouseRegistered = RegisterRawInputDevices(&mouseDevice, 1, sizeof(mouseDevice)) != FALSE;

	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);

	return TRUE;
}

// 4J Stu - These functions are referenced from the Windows Input library
void ClearGlobalText()
{
	// clear the global text
	memset(chGlobalText,0,256);
	memset(ui16GlobalText,0,512);
}

uint16_t *GetGlobalText()
{
	//copy the ch text to ui16
	char * pchBuffer=(char *)ui16GlobalText;
		for(int i=0;i<256;i++)
		{
			pchBuffer[i*2]=chGlobalText[i];
		}
	return ui16GlobalText;
}
void SeedEditBox()
{
	DialogBox(hMyInst, MAKEINTRESOURCE(IDD_SEED),
		g_hWnd, reinterpret_cast<DLGPROC>(DlgProc));
}


//---------------------------------------------------------------------------
LRESULT CALLBACK DlgProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		switch(wParam)
		{
		case IDOK:
			// Set the text
			GetDlgItemText(hWndDlg,IDC_EDIT,chGlobalText,256);
			EndDialog(hWndDlg, 0);
			return TRUE;
		}
		break;
	}

	return FALSE;
}

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect( g_hWnd, &rc );
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;
//app.DebugPrintf("width: %d, height: %d\n", width, height);
	width = g_iScreenWidth;
	height = g_iScreenHeight;
app.DebugPrintf("width: %d, height: %d\n", width, height);

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE( driverTypes );

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE( featureLevels );

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory( &sd, sizeof( sd ) );
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = g_pcDisplayMode != 2;

	// On hybrid laptops the display may be wired to the integrated GPU. Choose
	// the render adapter independently; Windows will present to the display.
	// DXGI 1.1 is available to the project's v110 toolset, so prefer hardware
	// with the most dedicated video memory and try the remaining adapters too.
	struct PCAdapterCandidate { IDXGIAdapter1 *adapter; SIZE_T dedicatedBytes; };
	std::vector<PCAdapterCandidate> adapters;
	IDXGIFactory1 *factory = NULL;
	hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void **>(&factory));
	if (SUCCEEDED(hr))
	{
		for (UINT index = 0; ; ++index)
		{
			IDXGIAdapter1 *adapter = NULL;
			if (factory->EnumAdapters1(index, &adapter) == DXGI_ERROR_NOT_FOUND) break;
			if (!adapter) break;
			DXGI_ADAPTER_DESC1 desc = {0};
			if (SUCCEEDED(adapter->GetDesc1(&desc)) && !(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
			{
				PCAdapterCandidate candidate = {adapter, desc.DedicatedVideoMemory};
				std::vector<PCAdapterCandidate>::iterator at = adapters.begin();
				while (at != adapters.end() && at->dedicatedBytes >= candidate.dedicatedBytes) ++at;
				adapters.insert(at, candidate);
			}
			else adapter->Release();
		}
		factory->Release();
	}
	hr = E_FAIL;
	for (size_t index = 0; index < adapters.size() && FAILED(hr); ++index)
	{
		hr = D3D11CreateDeviceAndSwapChain(adapters[index].adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL,
			createDeviceFlags, featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &sd,
			&g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
		if (SUCCEEDED(hr))
		{
			g_driverType = D3D_DRIVER_TYPE_HARDWARE;
			DXGI_ADAPTER_DESC1 selected = {0};
			if (SUCCEEDED(adapters[index].adapter->GetDesc1(&selected)))
				app.DebugPrintf("Selected GPU: %ls\n", selected.Description);
		}
	}
	for (size_t index = 0; index < adapters.size(); ++index) adapters[index].adapter->Release();

	// Retain the original default-hardware and software fallbacks if enumeration
	// or explicit hardware device creation fails.
	for (UINT driverTypeIndex = 0; FAILED(hr) && driverTypeIndex < numDriverTypes; ++driverTypeIndex)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, g_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
	}
	if( FAILED( hr ) )
		return hr;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = NULL;
	hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
	if( FAILED( hr ) )
		return hr;

	// Create a depth stencil buffer
	D3D11_TEXTURE2D_DESC descDepth;

	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = g_pd3dDevice->CreateTexture2D(&descDepth, NULL, &g_pDepthStencilBuffer);

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSView;
	descDSView.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDSView.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSView.Texture2D.MipSlice = 0;

	hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencilBuffer, &descDSView, &g_pDepthStencilView);

	hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_pRenderTargetView );
	pBackBuffer->Release();
	if( FAILED( hr ) )
		return hr;

	g_pImmediateContext->OMSetRenderTargets( 1, &g_pRenderTargetView, g_pDepthStencilView );

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports( 1, &vp );

	RenderManager.Initialise(g_pd3dDevice, g_pSwapChain);

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Render the frame
//--------------------------------------------------------------------------------------
void Render()
{
	// Just clear the backbuffer
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; //red,green,blue,alpha

	g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, ClearColor );
	g_pSwapChain->Present( 0, 0 );
}

//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
	if( g_pImmediateContext ) g_pImmediateContext->ClearState();

	if( g_pRenderTargetView ) g_pRenderTargetView->Release();
	if( g_pSwapChain ) { g_pSwapChain->SetFullscreenState(FALSE, NULL); g_pSwapChain->Release(); }
	if( g_pImmediateContext ) g_pImmediateContext->Release();
	if( g_pd3dDevice ) g_pd3dDevice->Release();
}



// Resolve game data independently of the directory from which the executable was launched.
bool PCSetResourceDirectory()
{
	wchar_t executableDir[MAX_PATH] = {0};
	DWORD len = GetModuleFileNameW(NULL, executableDir, MAX_PATH);
	if (len == 0 || len >= MAX_PATH) return false;
	wchar_t *slash = wcsrchr(executableDir, L'\\');
	if (!slash) return false;
	*slash = 0;
	wchar_t candidates[2][MAX_PATH];
	if (swprintf_s(candidates[0], MAX_PATH, L"%s", executableDir) < 0 ||
		swprintf_s(candidates[1], MAX_PATH, L"%s\\..\\..\\Minecraft.Client", executableDir) < 0) return false;
	for (int i = 0; i < 2; ++i)
	{
		wchar_t archive[MAX_PATH];
		if (swprintf_s(archive, MAX_PATH, L"%s\\Common\\Media\\MediaWindows64.arc", candidates[i]) < 0) continue;
		DWORD attributes = GetFileAttributesW(archive);
		if (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY) && SetCurrentDirectoryW(candidates[i])) return true;
	}
	return false;
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
					   _In_opt_ HINSTANCE hPrevInstance,
					   _In_ LPTSTR    lpCmdLine,
					   _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	// Without this, Windows may report DPI-virtualized desktop dimensions.
	SetProcessDPIAware();

	if (!PCSetResourceDirectory())
	{
		MessageBoxW(NULL, L"Game resources were not found. Keep the executable in its build folder alongside the Minecraft.Client resources.", L"Minecraft", MB_OK | MB_ICONERROR);
		return 1;
	}

	if(lpCmdLine)
	{
		if(lpCmdLine[0] == '1')
		{
			g_iScreenWidth = 1280;
			g_iScreenHeight = 720;
		}
		else if(lpCmdLine[0] == '2')
		{
			g_iScreenWidth = 640;
			g_iScreenHeight = 480;
		}
		else if(lpCmdLine[0] == '3')
		{
			// Vita
			g_iScreenWidth = 720;
			g_iScreenHeight = 408;

			// Vita native
			//g_iScreenWidth = 960;
			//g_iScreenHeight = 544;
		}
	}


	PCLoadGraphicsSettings();

	// Initialize global strings
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hMyInst=hInstance;

	if( FAILED( InitDevice() ) )
	{
		CleanupDevice();
		return 0;
	}

#if 0
	// Main message loop
	MSG msg = {0};
	while( WM_QUIT != msg.message )
	{
		if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else
		{
			Render();
		}
	}

	return (int) msg.wParam;
#endif

	static bool bTrialTimerDisplayed=true;

#ifdef MEMORY_TRACKING
	ResetMem();
	MEMORYSTATUS memStat;
	GlobalMemoryStatus(&memStat);
	printf("RESETMEM start: Avail. phys %d\n",memStat.dwAvailPhys/(1024*1024));
#endif

#if 0
	// Initialize D3D
	hr = InitD3D( &pDevice, &d3dpp );
	g_pD3DDevice = pDevice;
	if( FAILED(hr) )
	{
		app.DebugPrintf
			( "Failed initializing D3D.\n" );
		return -1;
	}

	// Initialize the application, assuming sharing of the d3d interface.
	hr = app.InitShared( pDevice, &d3dpp,
		XuiPNGTextureLoader );

	if ( FAILED(hr) )
	{
		app.DebugPrintf
			( "Failed initializing application.\n" );

		return -1;
	}

#endif
	app.loadMediaArchive();

	RenderManager.Initialise(g_pd3dDevice, g_pSwapChain);
	
	app.loadStringTable();
	ui.init(g_pd3dDevice,g_pImmediateContext,g_pRenderTargetView,g_pDepthStencilView,g_iScreenWidth,g_iScreenHeight);

	////////////////
	// Initialise //
	////////////////

	// Set the number of possible joypad layouts that the user can switch between, and the number of actions
	PCSonyPad::Initialize();
	// SDL may register raw-input devices during initialization. Ensure the
	// game's mouse registration still targets the game window afterwards.
	RAWINPUTDEVICE pcMouseDevice = {0};
	pcMouseDevice.usUsagePage = 1;
	pcMouseDevice.usUsage = 2;
	pcMouseDevice.hwndTarget = g_hWnd;
	g_pcRawMouseRegistered = RegisterRawInputDevices(&pcMouseDevice, 1, sizeof(pcMouseDevice)) != FALSE;
	InputManager.Initialise(1,3,MINECRAFT_ACTION_MAX, ACTION_MAX_MENU);

	// Set the default joypad action mappings for Minecraft
	DefineActions();
	InputManager.SetJoypadMapVal(0,0);
	InputManager.SetKeyRepeatRate(0.3f,0.2f);

	// Initialise the profile manager with the game Title ID, Offer ID, a profile version number, and the number of profile values and settings
	ProfileManager.Initialise(TITLEID_MINECRAFT,
		app.m_dwOfferID,
		PROFILE_VERSION_10,
		NUM_PROFILE_VALUES,
		NUM_PROFILE_SETTINGS,
		dwProfileSettingsA,
		app.GAME_DEFINED_PROFILE_DATA_BYTES*XUSER_MAX_COUNT,
		&app.uiGameDefinedDataChangedBitmask
		);
#if 0
	// register the awards
	ProfileManager.RegisterAward(eAward_TakingInventory,	ACHIEVEMENT_01, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_GettingWood,		ACHIEVEMENT_02, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_Benchmarking,		ACHIEVEMENT_03, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_TimeToMine,			ACHIEVEMENT_04, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_HotTopic,			ACHIEVEMENT_05, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_AquireHardware,		ACHIEVEMENT_06, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_TimeToFarm,			ACHIEVEMENT_07, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_BakeBread,			ACHIEVEMENT_08, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_TheLie,				ACHIEVEMENT_09, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_GettingAnUpgrade,	ACHIEVEMENT_10, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_DeliciousFish,		ACHIEVEMENT_11, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_OnARail,			ACHIEVEMENT_12, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_TimeToStrike,		ACHIEVEMENT_13, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_MonsterHunter,		ACHIEVEMENT_14, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_CowTipper,			ACHIEVEMENT_15, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_WhenPigsFly,		ACHIEVEMENT_16, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_LeaderOfThePack,	ACHIEVEMENT_17, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_MOARTools,			ACHIEVEMENT_18, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_DispenseWithThis,	ACHIEVEMENT_19, eAwardType_Achievement);
	ProfileManager.RegisterAward(eAward_InToTheNether,		ACHIEVEMENT_20, eAwardType_Achievement);

	ProfileManager.RegisterAward(eAward_mine100Blocks,		GAMER_PICTURE_GAMERPIC1,			eAwardType_GamerPic,false,app.GetStringTable(),IDS_AWARD_TITLE,IDS_AWARD_GAMERPIC1,IDS_CONFIRM_OK);
	ProfileManager.RegisterAward(eAward_kill10Creepers,		GAMER_PICTURE_GAMERPIC2,			eAwardType_GamerPic,false,app.GetStringTable(),IDS_AWARD_TITLE,IDS_AWARD_GAMERPIC2,IDS_CONFIRM_OK);

	ProfileManager.RegisterAward(eAward_eatPorkChop,		AVATARASSETAWARD_PORKCHOP_TSHIRT,	eAwardType_AvatarItem,false,app.GetStringTable(),IDS_AWARD_TITLE,IDS_AWARD_AVATAR1,IDS_CONFIRM_OK);
	ProfileManager.RegisterAward(eAward_play100Days,		AVATARASSETAWARD_WATCH,				eAwardType_AvatarItem,false,app.GetStringTable(),IDS_AWARD_TITLE,IDS_AWARD_AVATAR2,IDS_CONFIRM_OK);
	ProfileManager.RegisterAward(eAward_arrowKillCreeper,	AVATARASSETAWARD_CAP,				eAwardType_AvatarItem,false,app.GetStringTable(),IDS_AWARD_TITLE,IDS_AWARD_AVATAR3,IDS_CONFIRM_OK);

	ProfileManager.RegisterAward(eAward_socialPost,			0,									eAwardType_Theme,false,app.GetStringTable(),IDS_AWARD_TITLE,IDS_AWARD_THEME,IDS_CONFIRM_OK,THEME_NAME,THEME_FILESIZE);

	// Rich Presence init - number of presences, number of contexts
	ProfileManager.RichPresenceInit(4,1);
	ProfileManager.RegisterRichPresenceContext(CONTEXT_GAME_STATE);

	// initialise the storage manager with a default save display name, a Minimum save size, and a callback for displaying the saving message
	StorageManager.Init(app.GetString(IDS_DEFAULT_SAVENAME),"savegame.dat",FIFTY_ONE_MB,&CConsoleMinecraftApp::DisplaySavingMessage,(LPVOID)&app);
	// Set up the global title storage path
	StorageManager.StoreTMSPathName();

	// set a function to be called when there's a sign in change, so we can exit a level if the primary player signs out
	ProfileManager.SetSignInChangeCallback(&CConsoleMinecraftApp::SignInChangeCallback,(LPVOID)&app);

	// set a function to be called when the ethernet is disconnected, so we can back out if required
	ProfileManager.SetNotificationsCallback(&CConsoleMinecraftApp::NotificationsCallback,(LPVOID)&app);

#endif
	// Set a callback for the default player options to be set - when there is no profile data for the player
	ProfileManager.SetDefaultOptionsCallback(&CConsoleMinecraftApp::DefaultOptionsCallback,(LPVOID)&app);
#if 0
	// Set a callback to deal with old profile versions needing updated to new versions
	ProfileManager.SetOldProfileVersionCallback(&CConsoleMinecraftApp::OldProfileVersionCallback,(LPVOID)&app);

	// Set a callback for when there is a read error on profile data
	ProfileManager.SetProfileReadErrorCallback(&CConsoleMinecraftApp::ProfileReadErrorCallback,(LPVOID)&app);

#endif
	// QNet needs to be setup after profile manager, as we do not want its Notify listener to handle
	// XN_SYS_SIGNINCHANGED notifications. This does mean that we need to have a callback in the
	// ProfileManager for XN_LIVE_INVITE_ACCEPTED for QNet.
	g_NetworkManager.Initialise();



	// 4J-PB moved further down
	//app.InitGameSettings();

	// debug switch to trial version
	ProfileManager.SetDebugFullOverride(true);

#if 0
	//ProfileManager.AddDLC(2);
	StorageManager.SetDLCPackageRoot("DLCDrive");
	StorageManager.RegisterMarketplaceCountsCallback(&CConsoleMinecraftApp::MarketplaceCountsCallback,(LPVOID)&app);
	// Kinect !

	if(XNuiGetHardwareStatus()!=0)
	{
		// If the Kinect Sensor is not physically connected, this function returns 0.
		NuiInitialize(NUI_INITIALIZE_FLAG_USES_HIGH_QUALITY_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH |
			NUI_INITIALIZE_FLAG_EXTRAPOLATE_FLOOR_PLANE | NUI_INITIALIZE_FLAG_USES_FITNESS | NUI_INITIALIZE_FLAG_NUI_GUIDE_DISABLED | NUI_INITIALIZE_FLAG_SUPPRESS_AUTOMATIC_UI,NUI_INITIALIZE_DEFAULT_HARDWARE_THREAD );
	}

	// Sentient !
	hr = TelemetryManager->Init();

#endif
	// Initialise TLS for tesselator, for this main thread
	Tesselator::CreateNewThreadStorage(1024*1024);
	// Initialise TLS for AABB and Vec3 pools, for this main thread
	AABB::CreateNewThreadStorage();
	Vec3::CreateNewThreadStorage();
	IntCache::CreateNewThreadStorage();
	Compression::CreateNewThreadStorage();
	OldChunkStorage::CreateNewThreadStorage();
	Level::enableLightingCache();
	Tile::CreateNewThreadStorage();

	Minecraft::main();
	Minecraft *pMinecraft=Minecraft::GetInstance();
	PCApplyGraphicsQuality();

	app.InitGameSettings();

#if 0
	//bool bDisplayPauseMenu=false;

	// set the default gamma level
	float fVal=50.0f*327.68f;
	RenderManager.UpdateGamma((unsigned short)fVal);

	// load any skins
	//app.AddSkinsToMemoryTextureFiles();

	// set the achievement text for a trial achievement, now we have the string table loaded
	ProfileManager.SetTrialTextStringTable(app.GetStringTable(),IDS_CONFIRM_OK, IDS_CONFIRM_CANCEL);
	ProfileManager.SetTrialAwardText(eAwardType_Achievement,IDS_UNLOCK_TITLE,IDS_UNLOCK_ACHIEVEMENT_TEXT);
	ProfileManager.SetTrialAwardText(eAwardType_GamerPic,IDS_UNLOCK_TITLE,IDS_UNLOCK_GAMERPIC_TEXT);
	ProfileManager.SetTrialAwardText(eAwardType_AvatarItem,IDS_UNLOCK_TITLE,IDS_UNLOCK_AVATAR_TEXT);
	ProfileManager.SetTrialAwardText(eAwardType_Theme,IDS_UNLOCK_TITLE,IDS_UNLOCK_THEME_TEXT);
	ProfileManager.SetUpsellCallback(&app.UpsellReturnedCallback,&app);

	// Set up a debug character press sequence
#ifndef _FINAL_BUILD
	app.SetDebugSequence("LRLRYYY");
#endif

	// Initialise the social networking manager.
	CSocialManager::Instance()->Initialise();

	// Update the base scene quick selects now that the minecraft class exists
	//CXuiSceneBase::UpdateScreenSettings(0);
#endif
	app.InitialiseTips();
#if 0

	DWORD initData=0;

#ifndef _FINAL_BUILD
#ifndef _DEBUG
#pragma message(__LOC__"Need to define the _FINAL_BUILD before submission")
#endif
#endif

	// Set the default sound levels
	pMinecraft->options->set(Options::Option::MUSIC,1.0f);
	pMinecraft->options->set(Options::Option::SOUND,1.0f);

	app.NavigateToScene(XUSER_INDEX_ANY,eUIScene_Intro,&initData);
#endif

	// Set the default sound levels
	pMinecraft->options->set(Options::Option::MUSIC,1.0f);
	pMinecraft->options->set(Options::Option::SOUND,1.0f);

	//app.TemporaryCreateGameStart();

	//Sleep(10000);
#if 0
	// Intro loop ?
	while(app.IntroRunning())
	{
		ProfileManager.Tick();
		// Tick XUI
		app.RunFrame();

		// 4J : WESTY : Added to ensure we always have clear background for intro.
		RenderManager.SetClearColour(D3DCOLOR_RGBA(0,0,0,255));
		RenderManager.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Render XUI
		hr = app.Render();

		// Present the frame.
		RenderManager.Present();

		// Update XUI Timers
		hr = XuiTimersRun();
	}
#endif
	MSG msg = {0};
	while( WM_QUIT != msg.message )
	{
		if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
			continue;
		}
		DWORD pcFrameStart = GetTickCount();
		PCUpdateCursor(GetForegroundWindow() == g_hWnd);
		RenderManager.StartFrame();
#if 0
		if(pMinecraft->soundEngine->isStreamingWavebankReady() &&
			!pMinecraft->soundEngine->isPlayingStreamingGameMusic() &&
			!pMinecraft->soundEngine->isPlayingStreamingCDMusic() )
		{
			// play some music in the menus
			pMinecraft->soundEngine->playStreaming(L"", 0, 0, 0, 0, 0, false);
		}
#endif

		// 		static bool bPlay=false;
		// 		if(bPlay)
		// 		{
		// 			bPlay=false;
		// 			app.audio.PlaySound();
		// 		}

		app.UpdateTime();
		PIXBeginNamedEvent(0,"Input manager tick");
		PCSonyPad::Tick();
		InputManager.Tick();
		PCPollGamepad();
		PIXEndNamedEvent();
		PIXBeginNamedEvent(0,"Profile manager tick");
		//		ProfileManager.Tick();
		PIXEndNamedEvent();
		PIXBeginNamedEvent(0,"Storage manager tick");
		StorageManager.Tick();
		PIXEndNamedEvent();
		PIXBeginNamedEvent(0,"Render manager tick");
		RenderManager.Tick();
		PIXEndNamedEvent();

		// Tick the social networking manager.
		PIXBeginNamedEvent(0,"Social network manager tick");
		//		CSocialManager::Instance()->Tick();
		PIXEndNamedEvent();

		// Tick sentient.
		PIXBeginNamedEvent(0,"Sentient tick");
		MemSect(37);
		//		SentientManager.Tick();
		MemSect(0);
		PIXEndNamedEvent();

		PIXBeginNamedEvent(0,"Network manager do work #1");
		//		g_NetworkManager.DoWork();
		PIXEndNamedEvent();

		//		LeaderboardManager::Instance()->Tick();
		// Render game graphics.
		if(app.GetGameStarted())
		{
			pMinecraft->run_middle();
			app.SetAppPaused( g_NetworkManager.IsLocalGame() && g_NetworkManager.GetPlayerCount() == 1 && ui.IsPauseMenuDisplayed(ProfileManager.GetPrimaryPad()) );
		}
		else
		{
			MemSect(28);
			pMinecraft->soundEngine->tick(NULL, 0.0f);
			MemSect(0);
			pMinecraft->textures->tick(true,false);
			IntCache::Reset();
			if( app.GetReallyChangingSessionType() )
			{
				pMinecraft->tickAllConnections();		// Added to stop timing out when we are waiting after converting to an offline game
			}
		}

		pMinecraft->soundEngine->playMusicTick();

#ifdef MEMORY_TRACKING
		static bool bResetMemTrack = false;
		static bool bDumpMemTrack = false;

		MemPixStuff();

		if( bResetMemTrack )
		{
			ResetMem();
			MEMORYSTATUS memStat;
			GlobalMemoryStatus(&memStat);
			printf("RESETMEM: Avail. phys %d\n",memStat.dwAvailPhys/(1024*1024));
			bResetMemTrack = false;
		}

		if( bDumpMemTrack )
		{
			DumpMem();
			bDumpMemTrack = false;
			MEMORYSTATUS memStat;
			GlobalMemoryStatus(&memStat);
			printf("DUMPMEM: Avail. phys %d\n",memStat.dwAvailPhys/(1024*1024));
			printf("Renderer used: %d\n",RenderManager.CBuffSize(-1));
		}
#endif
#if 0
		static bool bDumpTextureUsage = false;
		if( bDumpTextureUsage )
		{
			RenderManager.TextureGetStats();
			bDumpTextureUsage = false;
		}
#endif
		ui.tick();
		ui.render();
#if 0
		app.HandleButtonPresses();

		// store the minecraft renderstates, and re-set them after the xui render
		GetRenderAndSamplerStates(pDevice,RenderStateA,SamplerStateA);

		// Tick XUI
		PIXBeginNamedEvent(0,"Xui running");
		app.RunFrame();
		PIXEndNamedEvent();

		// Render XUI

		PIXBeginNamedEvent(0,"XUI render");
		MemSect(7);
		hr = app.Render();
		MemSect(0);
		GetRenderAndSamplerStates(pDevice,RenderStateA2,SamplerStateA2);
		PIXEndNamedEvent();

		for(int i=0;i<8;i++)
		{
			if(RenderStateA2[i]!=RenderStateA[i])
			{
				//printf("Reseting RenderStateA[%d] after a XUI render\n",i);
				pDevice->SetRenderState(RenderStateModes[i],RenderStateA[i]);
			}
		}
		for(int i=0;i<5;i++)
		{
			if(SamplerStateA2[i]!=SamplerStateA[i])
			{
				//printf("Reseting SamplerStateA[%d] after a XUI render\n",i);
				pDevice->SetSamplerState(0,SamplerStateModes[i],SamplerStateA[i]);
			}
		}

		RenderManager.Set_matrixDirty();
#endif
		// Present the frame.
		RenderManager.Present();

		ui.CheckMenuDisplayed();
#if 0
		PIXBeginNamedEvent(0,"Profile load check");
		// has the game defined profile data been changed (by a profile load)
		if(app.uiGameDefinedDataChangedBitmask!=0)
		{
			void *pData;
			for(int i=0;i<XUSER_MAX_COUNT;i++)
			{
				if(app.uiGameDefinedDataChangedBitmask&(1<<i))
				{\
				// It has - game needs to update its values with the data from the profile
				pData=ProfileManager.GetGameDefinedProfileData(i);
				// reset the changed flag
				app.ClearGameSettingsChangedFlag(i);
				app.DebugPrintf("***  - APPLYING GAME SETTINGS CHANGE for pad %d\n",i);
				app.ApplyGameSettingsChanged(i);

#ifdef _DEBUG_MENUS_ENABLED
				if(app.DebugSettingsOn())
				{
					app.ActionDebugMask(i);
				}
				else
				{
					// force debug mask off
					app.ActionDebugMask(i,true);
				}
#endif
				// clear the stats first - there could have beena signout and sign back in in the menus
				// need to clear the player stats - can't assume it'll be done in setlevel - we may not be in the game
				pMinecraft->stats[ i ]->clear();
				pMinecraft->stats[i]->parse(pData);
				}
			}

			// Check to see if we can post to social networks.
			CSocialManager::Instance()->RefreshPostingCapability();

			// clear the flag
			app.uiGameDefinedDataChangedBitmask=0;

			// Check if any profile write are needed
			app.CheckGameSettingsChanged();
		}
		PIXEndNamedEvent();
		app.TickDLCOffersRetrieved();
		app.TickTMSPPFilesRetrieved();

		PIXBeginNamedEvent(0,"Network manager do work #2");
		g_NetworkManager.DoWork();
		PIXEndNamedEvent();

		PIXBeginNamedEvent(0,"Misc extra xui");
		// Update XUI Timers
		hr = XuiTimersRun();

#endif
		// Any threading type things to deal with from the xui side?
		app.HandleXuiActions();

#if 0
		PIXEndNamedEvent();
#endif

		// 4J-PB - Update the trial timer display if we are in the trial version
		if(!ProfileManager.IsFullVersion())
		{
			// display the trial timer
			if(app.GetGameStarted())
			{
				// 4J-PB - if the game is paused, add the elapsed time to the trial timer count so it doesn't tick down
				if(app.IsAppPaused())
				{
					app.UpdateTrialPausedTimer();
				}
				ui.UpdateTrialTimer(ProfileManager.GetPrimaryPad());
			}
		}
		else
		{
			// need to turn off the trial timer if it was on , and we've unlocked the full version
			if(bTrialTimerDisplayed)
			{
				ui.ShowTrialTimer(false);
				bTrialTimerDisplayed=false;
			}
		}

		// Fix for #7318 - Title crashes after short soak in the leaderboards menu
		// A memory leak was caused because the icon renderer kept creating new Vec3's because the pool wasn't reset
		Vec3::resetPool();
		// Menus do not need hundreds of redraws per second. Keep active gameplay
		// uncapped, while also avoiding a busy loop when the window is inactive.
		DWORD frameBudget = GetForegroundWindow() != g_hWnd ? 33 : (!app.GetGameStarted() ? 16 : 0);
		DWORD frameElapsed = GetTickCount() - pcFrameStart;
		if (frameElapsed < frameBudget) Sleep(frameBudget - frameElapsed);
	}

	PCSonyPad::Shutdown();
	// Free resources, unregister custom classes, and exit.
	//	app.Uninit();
	g_pd3dDevice->Release();
}

#ifdef MEMORY_TRACKING

int totalAllocGen = 0;
unordered_map<int,int> allocCounts;
bool trackEnable = false;
bool trackStarted = false;
volatile size_t sizeCheckMin = 1160;
volatile size_t sizeCheckMax = 1160;
volatile int sectCheck = 48;
CRITICAL_SECTION memCS;
DWORD tlsIdx;

LPVOID XMemAlloc(SIZE_T dwSize, DWORD dwAllocAttributes)
{
	if( !trackStarted )
	{
		void *p = XMemAllocDefault(dwSize,dwAllocAttributes);
		size_t realSize = XMemSizeDefault(p, dwAllocAttributes);
		totalAllocGen += realSize;
		return p;
	}

	EnterCriticalSection(&memCS);

	void *p=XMemAllocDefault(dwSize + 16,dwAllocAttributes);
	size_t realSize = XMemSizeDefault(p,dwAllocAttributes) - 16;

	if( trackEnable )
	{
#if 1
		int sect = ((int) TlsGetValue(tlsIdx)) & 0x3f;
		*(((unsigned char *)p)+realSize) = sect;

		if( ( realSize >= sizeCheckMin ) && ( realSize <= sizeCheckMax ) && ( ( sect == sectCheck ) || ( sectCheck == -1 ) ) )
		{
			app.DebugPrintf("Found one\n");
		}
#endif

		if( p )
		{
			totalAllocGen += realSize;
			trackEnable = false;
			int key = ( sect << 26 ) | realSize;
			int oldCount = allocCounts[key];
			allocCounts[key] = oldCount + 1;

			trackEnable = true;
		}
	}

	LeaveCriticalSection(&memCS);

	return p;
}

void* operator new (size_t size)
{
	return (unsigned char *)XMemAlloc(size,MAKE_XALLOC_ATTRIBUTES(0,FALSE,TRUE,FALSE,0,XALLOC_PHYSICAL_ALIGNMENT_DEFAULT,XALLOC_MEMPROTECT_READWRITE,FALSE,XALLOC_MEMTYPE_HEAP));
}

void operator delete (void *p)
{
	XMemFree(p,MAKE_XALLOC_ATTRIBUTES(0,FALSE,TRUE,FALSE,0,XALLOC_PHYSICAL_ALIGNMENT_DEFAULT,XALLOC_MEMPROTECT_READWRITE,FALSE,XALLOC_MEMTYPE_HEAP));
}

void WINAPI XMemFree(PVOID pAddress, DWORD dwAllocAttributes)
{
	bool special = false;
	if( dwAllocAttributes == 0 )
	{
		dwAllocAttributes = MAKE_XALLOC_ATTRIBUTES(0,FALSE,TRUE,FALSE,0,XALLOC_PHYSICAL_ALIGNMENT_DEFAULT,XALLOC_MEMPROTECT_READWRITE,FALSE,XALLOC_MEMTYPE_HEAP);
		special = true;
	}
	if(!trackStarted )
	{
		size_t realSize = XMemSizeDefault(pAddress, dwAllocAttributes);
		XMemFreeDefault(pAddress, dwAllocAttributes);
		totalAllocGen -= realSize;
		return;
	}
	EnterCriticalSection(&memCS);
	if( pAddress )
	{
		size_t realSize = XMemSizeDefault(pAddress, dwAllocAttributes) - 16;

		if(trackEnable)
		{
			int sect = *(((unsigned char *)pAddress)+realSize);
			totalAllocGen -= realSize;
			trackEnable = false;
			int key = ( sect << 26 ) | realSize;
			int oldCount = allocCounts[key];
			allocCounts[key] = oldCount - 1;
			trackEnable = true;
		}
		XMemFreeDefault(pAddress, dwAllocAttributes);
	}
	LeaveCriticalSection(&memCS);
}

SIZE_T WINAPI XMemSize(
	PVOID pAddress,
	DWORD dwAllocAttributes
	)
{
	if( trackStarted )
	{
		return XMemSizeDefault(pAddress, dwAllocAttributes) - 16;
	}
	else
	{
		return XMemSizeDefault(pAddress, dwAllocAttributes);
	}
}

void DumpMem()
{
	int totalLeak = 0;
	for(AUTO_VAR(it, allocCounts.begin()); it != allocCounts.end(); it++ )
	{
		if(it->second > 0 )
		{
			app.DebugPrintf("%d %d %d %d\n",( it->first >> 26 ) & 0x3f,it->first & 0x03ffffff, it->second, (it->first & 0x03ffffff) * it->second);
			totalLeak += ( it->first & 0x03ffffff ) * it->second;
		}
	}
	app.DebugPrintf("Total %d\n",totalLeak);
}

void ResetMem()
{
	if( !trackStarted )
	{
		trackEnable = true;
		trackStarted = true;
		totalAllocGen = 0;
		InitializeCriticalSection(&memCS);
		tlsIdx = TlsAlloc();
	}
	EnterCriticalSection(&memCS);
	trackEnable = false;
	allocCounts.clear();
	trackEnable = true;
	LeaveCriticalSection(&memCS);
}

void MemSect(int section)
{
	unsigned int value = (unsigned int)TlsGetValue(tlsIdx);
	if( section == 0 ) // pop
	{
		value = (value >> 6) & 0x03ffffff;
	}
	else
	{
		value = (value << 6) | section;
	}
	TlsSetValue(tlsIdx, (LPVOID)value);
}

void MemPixStuff()
{
	const int MAX_SECT = 46;

	int totals[MAX_SECT] = {0};

	for(AUTO_VAR(it, allocCounts.begin()); it != allocCounts.end(); it++ )
	{
		if(it->second > 0 )
		{
			int sect = ( it->first >> 26 ) & 0x3f;
			int bytes = it->first & 0x03ffffff;
			totals[sect] += bytes * it->second;
		}
	}

	unsigned int allSectsTotal = 0;
	for( int i = 0; i < MAX_SECT; i++ )
	{
		allSectsTotal += totals[i];
		PIXAddNamedCounter(((float)totals[i])/1024.0f,"MemSect%d",i);
	}

	PIXAddNamedCounter(((float)allSectsTotal)/(4096.0f),"MemSect total pages");
}

#endif