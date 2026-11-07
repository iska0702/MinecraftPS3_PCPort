#include "stdafx.h"
#include "Minecraft.h"
#include "GameMode.h"
#include "..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.level.storage.h"
#include "Input.h"
#include "..\Minecraft.Client\LocalPlayer.h"
#include "Options.h"

Input::Input()
{
	xa = 0;
	ya = 0;
	wasJumping = false;
	jumping = false;
	sneaking = false;

	lReset = false;
    rReset = false;
}

void Input::tick(LocalPlayer *player)
{
	// 4J Stu -  Assume that we only need one input class, even though the java has subclasses for keyboard/controller
	// This function is based on the ControllerInput class in the Java, and will probably need changed
	//OutputDebugString("INPUT: Beginning input tick\n");

	Minecraft *pMinecraft=Minecraft::GetInstance();
	int iPad=player->GetXboxPad();

	// 4J-PB minecraft movement seems to be the wrong way round, so invert x!
	if( pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_LEFT) || pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_RIGHT) )
		xa = -InputManager.GetJoypadStick_LX(iPad);
	else
		xa = 0.0f;
	
	if( pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_FORWARD) || pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_BACKWARD) )
		ya = InputManager.GetJoypadStick_LY(iPad);
	else
		ya = 0.0f;

#ifndef _CONTENT_PACKAGE
	if (app.GetFreezePlayers())
	{
		xa = ya = 0.0f;
		player->abilities.flying = true;
	}
#endif
	
    if (!lReset)
    {
        if (xa*xa+ya*ya==0.0f)
        {
            lReset = true;
        }
        xa = ya = 0.0f;
    }

#ifdef _WINDOWS64
	// The primary player can also move with the keyboard while the game has focus.
	extern HWND g_hWnd;
	bool keyboardActive = iPad == 0 && GetForegroundWindow() == g_hWnd && !ui.GetMenuDisplayed(iPad);
	if (keyboardActive
#ifndef _CONTENT_PACKAGE
		&& !app.GetFreezePlayers()
#endif
	)
	{
		if (pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_LEFT) && (GetAsyncKeyState('A') & 0x8000)) xa += 1.0f;
		if (pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_RIGHT) && (GetAsyncKeyState('D') & 0x8000)) xa -= 1.0f;
		if (pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_FORWARD) && (GetAsyncKeyState('W') & 0x8000)) ya += 1.0f;
		if (pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_BACKWARD) && (GetAsyncKeyState('S') & 0x8000)) ya -= 1.0f;
		if (xa > 1.0f) xa = 1.0f;
		if (xa < -1.0f) xa = -1.0f;
		if (ya > 1.0f) ya = 1.0f;
		if (ya < -1.0f) ya = -1.0f;
	}
#endif

	// 4J - in flying mode, don't actually toggle sneaking
	if(!player->abilities.flying)
	{
		if((player->ullButtonsPressed&(1LL<<MINECRAFT_ACTION_SNEAK_TOGGLE)) && pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_SNEAK_TOGGLE))
		{
			sneaking=!sneaking;
		}
	}

#ifdef _WINDOWS64
	if (keyboardActive) sneaking = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
#endif
	if(sneaking)
	{
		xa*=0.3f;
		ya*=0.3f;
	}

    float turnSpeed = 50.0f;

	float tx = 0.0f;
	float ty = 0.0f;
	if( pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_LOOK_LEFT) || pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_LOOK_RIGHT) )
		tx = InputManager.GetJoypadStick_RX(iPad)*(((float)app.GetGameSettings(iPad,eGameSetting_Sensitivity_InGame))/100.0f); // apply sensitivity to look
	if( pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_LOOK_UP) || pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_LOOK_DOWN) )
		ty = InputManager.GetJoypadStick_RY(iPad)*(((float)app.GetGameSettings(iPad,eGameSetting_Sensitivity_InGame))/100.0f); // apply sensitivity to look
	
#ifndef _CONTENT_PACKAGE
	if (app.GetFreezePlayers())	tx = ty = 0.0f;
#endif

	// 4J: WESTY : Invert look Y if required.
	if ( app.GetGameSettings(iPad,eGameSetting_ControlInvertLook) )
	{
		ty = -ty;
	}

    if (!rReset)
    {
        if (tx*tx+ty*ty==0.0f)
        {
            rReset = true;
        }
        tx = ty = 0.0f;
    }
	player->interpolateTurn(tx * abs(tx) * turnSpeed, ty * abs(ty) * turnSpeed);
#ifdef _WINDOWS64
	// Use relative cursor movement while playing; leave the cursor alone in menus.
	static bool mouseCaptured = false;
	extern bool g_pcUsingGamepad;
	extern int g_pcMouseSensitivity, g_pcMouseInvert;
	if (keyboardActive && !g_pcUsingGamepad)
	{
		RECT client; POINT center, cursor;
		GetClientRect(g_hWnd, &client);
		center.x = (client.left + client.right) / 2;
		center.y = (client.top + client.bottom) / 2;
		ClientToScreen(g_hWnd, &center);
		if (mouseCaptured && GetCursorPos(&cursor))
		{
			float dx = (float)(cursor.x - center.x) * g_pcMouseSensitivity / 100.0f;
			float dy = (float)(center.y - cursor.y) * g_pcMouseSensitivity / 100.0f;
			if (!pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_LOOK_LEFT)) dx = 0;
			if (!pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_LOOK_UP)) dy = 0;
			if (g_pcMouseInvert) dy = -dy;
			player->interpolateTurn(dx, dy);
		}
		if (!mouseCaptured || cursor.x != center.x || cursor.y != center.y) SetCursorPos(center.x, center.y);
		mouseCaptured = true;
	}
	else mouseCaptured = false;
#endif
        
    //jumping = controller.isButtonPressed(0);

	
	unsigned int jump = InputManager.GetValue(iPad, MINECRAFT_ACTION_JUMP);
	#ifdef _WINDOWS64
	if (keyboardActive && (GetAsyncKeyState(VK_SPACE) & 0x8000)) jump = 1;
#endif
	if( jump > 0 && pMinecraft->localgameModes[iPad]->isInputAllowed(MINECRAFT_ACTION_JUMP) )
		jumping = true;
	else
 		jumping = false;

#ifndef _CONTENT_PACKAGE
	if (app.GetFreezePlayers())	jumping = false;
#endif

	//OutputDebugString("INPUT: End input tick\n");
}