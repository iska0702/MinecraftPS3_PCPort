#include "stdafx.h"
#include "UI.h"
#include "UIScene_SettingsMenu.h"
#include "..\..\Minecraft.h"
#ifdef _WINDOWS64
extern int g_pcDisplayMode, g_pcOutputWidth, g_pcOutputHeight, g_pcRenderScale, g_pcGraphicsQuality, g_pcAspectRatio, g_pcViewDistance;
extern int g_pcAspectCount;
extern void PCGetCanvasSize(int &width, int &height);
extern void PCApplyGraphicsQuality();
extern void PCNextResolution();
extern void PCSaveGraphicsInt(const wchar_t *key, int value);
#endif

UIScene_SettingsMenu::UIScene_SettingsMenu(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
#ifdef _WINDOWS64
	m_pcGraphics = false;
	m_pcGraphicsExtra = false;
	m_pcControls = initData == (void *)1;
	m_pcControlsPage = 0;
#endif
	// Setup all the Iggy references we need for this scene
	initialiseMovie();
	
	bool bNotInGame=(Minecraft::GetInstance()->level==NULL);

	m_buttons[BUTTON_ALL_OPTIONS].init(app.GetString(IDS_OPTIONS),BUTTON_ALL_OPTIONS);
	m_buttons[BUTTON_ALL_AUDIO].init(app.GetString(IDS_AUDIO),BUTTON_ALL_AUDIO);
	m_buttons[BUTTON_ALL_CONTROL].init(app.GetString(IDS_CONTROL),BUTTON_ALL_CONTROL);
	m_buttons[BUTTON_ALL_GRAPHICS].init(app.GetString(IDS_GRAPHICS),BUTTON_ALL_GRAPHICS);
	m_buttons[BUTTON_ALL_UI].init(app.GetString(IDS_USER_INTERFACE),BUTTON_ALL_UI);
	m_buttons[BUTTON_ALL_RESETTODEFAULTS].init(app.GetString(IDS_RESET_TO_DEFAULTS),BUTTON_ALL_RESETTODEFAULTS);
#ifdef _WINDOWS64
	if (m_pcControls) updatePCControlsMenu();
#endif

	if(ProfileManager.GetPrimaryPad()!=m_iPad)
	{
		removeControl( &m_buttons[BUTTON_ALL_AUDIO], true);
		removeControl( &m_buttons[BUTTON_ALL_GRAPHICS], true);
	}

	doHorizontalResizeCheck();

	if(app.GetLocalPlayerCount()>1)
	{
#if TO_BE_IMPLEMENTED
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad,false);
#endif
	}
}

UIScene_SettingsMenu::~UIScene_SettingsMenu()
{
}

wstring UIScene_SettingsMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"SettingsMenuSplit";
	}
	else
	{
		return L"SettingsMenu";
	}
}

void UIScene_SettingsMenu::handleReload()
{
#ifdef _WINDOWS64
	if (m_pcControls) { updatePCControlsMenu(); doHorizontalResizeCheck(); return; }
	if (m_pcGraphics) { updatePCGraphicsMenu(); doHorizontalResizeCheck(); return; }
#endif
	if(ProfileManager.GetPrimaryPad()!=m_iPad)
	{
		removeControl( &m_buttons[BUTTON_ALL_AUDIO], true);
		removeControl( &m_buttons[BUTTON_ALL_GRAPHICS], true);
	}

	doHorizontalResizeCheck();
}

void UIScene_SettingsMenu::updateTooltips()
{
	ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
}

void UIScene_SettingsMenu::updateComponents()
{
	bool bNotInGame=(Minecraft::GetInstance()->level==NULL);
	if(bNotInGame)
	{
		m_parentLayer->showComponent(m_iPad,eUIComponent_Panorama,true);
		m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,true);
	}
	else
	{
		m_parentLayer->showComponent(m_iPad,eUIComponent_Panorama,false);
		
		if( app.GetLocalPlayerCount() == 1 ) m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,true);
		else m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,false);

	}
}

void UIScene_SettingsMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	//app.DebugPrintf("UIScene_DebugOverlay handling input for pad %d, key %d, down- %s, pressed- %s, released- %s\n", iPad, key, down?"TRUE":"FALSE", pressed?"TRUE":"FALSE", released?"TRUE":"FALSE");
	ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

	switch(key)
	{
	case ACTION_MENU_CANCEL:
		if(pressed)
		{
#ifdef _WINDOWS64
			if (m_pcGraphicsExtra) { m_pcGraphicsExtra = false; updatePCGraphicsMenu(); handled = true; break; }
			if (m_pcGraphics) { m_pcGraphics = false; updateStandardSettingsMenu(); handled = true; break; }
#endif
			// if the profile data has been changed, then force a profile write
			// It seems we're allowed to break the 5 minute rule if it's the result of a user action

			app.CheckGameSettingsChanged(true,iPad);          
			navigateBack();
		}
		break;
	case ACTION_MENU_OK:
#ifdef __ORBIS__
	case ACTION_MENU_TOUCHPAD_PRESS:
#endif
		sendInputToMovie(key, repeat, pressed, released);
		break;
#ifdef _WINDOWS64
	case ACTION_MENU_LEFT:
	case ACTION_MENU_RIGHT:
		if (m_pcControls && pressed)
		{
			m_pcControlsPage = key == ACTION_MENU_RIGHT ? 1 : 0;
			updatePCControlsMenu(); handled = true;
		}
		else if (m_pcGraphics && pressed)
		{
			m_pcGraphicsExtra = key == ACTION_MENU_RIGHT;
			updatePCGraphicsMenu(); handled = true;
		}
		break;
#endif
	case ACTION_MENU_UP:
	case ACTION_MENU_DOWN:
		sendInputToMovie(key, repeat, pressed, released);
		break;
	}
}

void UIScene_SettingsMenu::handlePress(F64 controlId, F64 childId)
{
	//CD - Added for audio
	ui.PlayUISFX(eSFX_Press);
#ifdef _WINDOWS64
	if (m_pcControls)
	{
		if ((int)controlId == BUTTON_ALL_RESETTODEFAULTS) { m_pcControlsPage = 1 - m_pcControlsPage; updatePCControlsMenu(); }
		return;
	}
	if (m_pcGraphics)
	{
		const int scales[] = {50, 75, 100, 125};
		if (m_pcGraphicsExtra)
		{
			int v;
			switch ((int)controlId)
			{
			case BUTTON_ALL_OPTIONS:
				v = app.GetGameSettings(m_iPad,eGameSetting_Gamma); app.SetGameSettings(m_iPad,eGameSetting_Gamma, v >= 100 ? 0 : min(v + 10, 100)); break;
			case BUTTON_ALL_AUDIO:
				v = app.GetGameSettings(m_iPad,eGameSetting_InterfaceOpacity); app.SetGameSettings(m_iPad,eGameSetting_InterfaceOpacity, v >= 100 ? 0 : min(v + 10, 100)); break;
			case BUTTON_ALL_CONTROL:
				v = app.GetGameSettings(m_iPad,eGameSetting_BedrockFog); app.SetGameSettings(m_iPad,eGameSetting_BedrockFog,!v); break;
			case BUTTON_ALL_GRAPHICS:
				v = app.GetGameSettings(m_iPad,eGameSetting_CustomSkinAnim); app.SetGameSettings(m_iPad,eGameSetting_CustomSkinAnim,!v); break;
			case BUTTON_ALL_UI:
				v = app.GetGameSettings(m_iPad,eGameSetting_UISize); app.SetGameSettings(m_iPad,eGameSetting_UISize,(v + 1) % 3); break;
			case BUTTON_ALL_RESETTODEFAULTS:
				g_pcViewDistance = (g_pcViewDistance + 1) % 4; PCSaveGraphicsInt(L"ViewDistance", g_pcViewDistance); PCApplyGraphicsQuality(); break;
			}
			updatePCGraphicsMenu(); return;
		}
		switch ((int)controlId)
		{
		case BUTTON_ALL_OPTIONS:
			g_pcDisplayMode = (g_pcDisplayMode + 1) % 3;
			PCSaveGraphicsInt(L"Mode", g_pcDisplayMode);
			break;
		case BUTTON_ALL_AUDIO:
			if (g_pcDisplayMode != 1)
			{
				PCNextResolution();
				PCSaveGraphicsInt(L"Width", g_pcOutputWidth); PCSaveGraphicsInt(L"Height", g_pcOutputHeight);
			}
			break;
		case BUTTON_ALL_CONTROL:
			do { g_pcAspectRatio = (g_pcAspectRatio + 1) % g_pcAspectCount; }
			while (g_pcAspectRatio == 2 || g_pcAspectRatio == 3);
			PCSaveGraphicsInt(L"AspectRatio", g_pcAspectRatio);
			break;
		case BUTTON_ALL_GRAPHICS:
			{
				int i = 0; while (i < 4 && scales[i] != g_pcRenderScale) ++i;
				g_pcRenderScale = scales[(i + 1) % 4]; PCSaveGraphicsInt(L"RenderScale", g_pcRenderScale);
			}
			break;
		case BUTTON_ALL_UI:
			g_pcGraphicsQuality = (g_pcGraphicsQuality + 1) % 3;
			PCSaveGraphicsInt(L"Quality", g_pcGraphicsQuality); PCApplyGraphicsQuality();
			break;
		case BUTTON_ALL_RESETTODEFAULTS:
			{
				int v = app.GetGameSettings(m_iPad,eGameSetting_Clouds);
				app.SetGameSettings(m_iPad,eGameSetting_Clouds,!v);
			}
			break;
		}
		updatePCGraphicsMenu();
		return;
	}
#endif

	switch((int)controlId)
	{
	case BUTTON_ALL_OPTIONS:
		ui.NavigateToScene(m_iPad, eUIScene_SettingsOptionsMenu);
		break;
	case BUTTON_ALL_AUDIO:
		ui.NavigateToScene(m_iPad, eUIScene_SettingsAudioMenu);
		break;
	case BUTTON_ALL_CONTROL:
		ui.NavigateToScene(m_iPad, eUIScene_SettingsControlMenu);
		break;
	case BUTTON_ALL_GRAPHICS:
#ifdef _WINDOWS64
		m_pcGraphics = true;
		updatePCGraphicsMenu();
#else
		ui.NavigateToScene(m_iPad, eUIScene_SettingsGraphicsMenu);
#endif
		break;
	case BUTTON_ALL_UI:
		ui.NavigateToScene(m_iPad, eUIScene_SettingsUIMenu);
		break;
	case BUTTON_ALL_RESETTODEFAULTS:
		{
			// check they really want to do this
			UINT uiIDA[2];
			uiIDA[0]=IDS_CONFIRM_CANCEL;
			uiIDA[1]=IDS_CONFIRM_OK;

			ui.RequestMessageBox(IDS_DEFAULTS_TITLE, IDS_DEFAULTS_TEXT, uiIDA, 2, m_iPad,&UIScene_SettingsMenu::ResetDefaultsDialogReturned,this, app.GetStringTable(), NULL, 0, false);
		}	
		break;
	}
}

int UIScene_SettingsMenu::ResetDefaultsDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	UIScene_SettingsMenu* pClass = (UIScene_SettingsMenu*)pParam;

	// results switched for this dialog
	if(result==C4JStorage::EMessage_ResultDecline) 
	{
#if (defined __PS3__ || defined __ORBIS__ || defined _DURANGO || defined __PSVITA__)
		app.SetDefaultOptions(StorageManager.GetDashboardProfileSettings(pClass->m_iPad),pClass->m_iPad);
#else
		app.SetDefaultOptions(ProfileManager.GetDashboardProfileSettings(pClass->m_iPad),pClass->m_iPad);
#endif
		// if the profile data has been changed, then force a profile write
		// It seems we're allowed to break the 5 minute rule if it's the result of a user action
		app.CheckGameSettingsChanged(true,iPad);
	}
	return 0;
}

#ifdef _WINDOWS64
void UIScene_SettingsMenu::updatePCGraphicsMenu()
{
	wchar_t label[256];
	const int ids[] = {BUTTON_ALL_OPTIONS, BUTTON_ALL_AUDIO, BUTTON_ALL_CONTROL, BUTTON_ALL_GRAPHICS, BUTTON_ALL_UI, BUTTON_ALL_RESETTODEFAULTS};
	for (int i = 0; i < 6; ++i) m_buttons[ids[i]].setEnable(true);
	if (m_pcGraphicsExtra)
	{
		swprintf_s(label, 256, L"Brightness: %d%%", app.GetGameSettings(m_iPad,eGameSetting_Gamma)); m_buttons[BUTTON_ALL_OPTIONS].setLabel(label);
		swprintf_s(label, 256, L"Interface opacity: %d%%", app.GetGameSettings(m_iPad,eGameSetting_InterfaceOpacity)); m_buttons[BUTTON_ALL_AUDIO].setLabel(label);
		m_buttons[BUTTON_ALL_CONTROL].setLabel(app.GetGameSettings(m_iPad,eGameSetting_BedrockFog) ? L"Bedrock fog: On" : L"Bedrock fog: Off");
		m_buttons[BUTTON_ALL_GRAPHICS].setLabel(app.GetGameSettings(m_iPad,eGameSetting_CustomSkinAnim) ? L"Skin animations: On" : L"Skin animations: Off");
		swprintf_s(label, 256, L"HUD size: %d", app.GetGameSettings(m_iPad,eGameSetting_UISize) + 1); m_buttons[BUTTON_ALL_UI].setLabel(label);
		swprintf_s(label, 256, L"Render distance: %d chunks (Left: page 1)", 16 >> g_pcViewDistance); m_buttons[BUTTON_ALL_RESETTODEFAULTS].setLabel(label);
		doHorizontalResizeCheck(); return;
	}
	const wchar_t *modes[] = {L"Windowed", L"Borderless", L"Fullscreen"};
	swprintf_s(label, 256, L"Display mode: %s", modes[g_pcDisplayMode]);
	m_buttons[BUTTON_ALL_OPTIONS].setLabel(label);
	int canvasWidth, canvasHeight; PCGetCanvasSize(canvasWidth, canvasHeight);
	if (g_pcDisplayMode == 1)
		swprintf_s(label, 256, L"Resolution: Desktop (%d x %d)", GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	else
		swprintf_s(label, 256, L"Resolution: %d x %d", g_pcOutputWidth, g_pcOutputHeight);
	m_buttons[BUTTON_ALL_AUDIO].setLabel(label);
	const wchar_t *ratios[] = {L"Auto", L"4:3", L"5:4", L"3:2", L"16:10", L"16:9", L"21:9", L"32:9", L"32:10", L"1:1", L"3:4", L"9:16", L"18:9", L"20:9", L"9:21"};
	swprintf_s(label, 256, L"Aspect ratio: %s", ratios[g_pcAspectRatio]);
	m_buttons[BUTTON_ALL_CONTROL].setLabel(label);
	swprintf_s(label, 256, L"Render scale: %d%%", g_pcRenderScale);
	m_buttons[BUTTON_ALL_GRAPHICS].setLabel(label);
	const wchar_t *quality[] = {L"Low", L"Medium", L"High"};
	swprintf_s(label, 256, L"Graphics quality: %s", quality[g_pcGraphicsQuality]);
	m_buttons[BUTTON_ALL_UI].setLabel(label);
	m_buttons[BUTTON_ALL_RESETTODEFAULTS].setLabel(app.GetGameSettings(m_iPad,eGameSetting_Clouds) ? L"Clouds: On (Right: page 2)" : L"Clouds: Off (Right: page 2)");
	doHorizontalResizeCheck();
}

void UIScene_SettingsMenu::updatePCControlsMenu()
{
	const int ids[] = {BUTTON_ALL_OPTIONS, BUTTON_ALL_AUDIO, BUTTON_ALL_CONTROL, BUTTON_ALL_GRAPHICS, BUTTON_ALL_UI, BUTTON_ALL_RESETTODEFAULTS};
	const wchar_t *page1[] = {L"Move: W A S D", L"Look: Mouse", L"Jump: Space", L"Sneak: Shift", L"Sprint: Ctrl", L"Controls 1/2 - Right: next"};
	const wchar_t *page2[] = {L"Mine / Attack: Left mouse", L"Use / Place: Right mouse", L"Inventory: E   Craft: C", L"Drop: Q   Pause: Esc", L"Hotbar: 1-9 / Mouse wheel", L"Controls 2/2 - Left: back"};
	for (int i = 0; i < 6; ++i) m_buttons[ids[i]].setLabel(m_pcControlsPage ? page2[i] : page1[i]);
	doHorizontalResizeCheck();
}

void UIScene_SettingsMenu::updateStandardSettingsMenu()
{
	const int ids[] = {BUTTON_ALL_OPTIONS, BUTTON_ALL_AUDIO, BUTTON_ALL_CONTROL, BUTTON_ALL_GRAPHICS, BUTTON_ALL_UI, BUTTON_ALL_RESETTODEFAULTS};
	for (int i = 0; i < 6; ++i) m_buttons[ids[i]].setEnable(true);
	m_buttons[BUTTON_ALL_OPTIONS].setLabel(app.GetString(IDS_OPTIONS));
	m_buttons[BUTTON_ALL_AUDIO].setLabel(app.GetString(IDS_AUDIO));
	m_buttons[BUTTON_ALL_CONTROL].setLabel(app.GetString(IDS_CONTROL));
	m_buttons[BUTTON_ALL_GRAPHICS].setLabel(app.GetString(IDS_GRAPHICS));
	m_buttons[BUTTON_ALL_UI].setLabel(app.GetString(IDS_USER_INTERFACE));
	m_buttons[BUTTON_ALL_RESETTODEFAULTS].setLabel(app.GetString(IDS_RESET_TO_DEFAULTS));
	doHorizontalResizeCheck();
}
#endif
