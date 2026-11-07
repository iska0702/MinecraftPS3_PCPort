// SDL3-backed PlayStation gamepads for the legacy XInput-based input library.
// SDL is loaded dynamically; failure leaves native XInput unchanged.
namespace PCSonyPad
{
    typedef bool (__cdecl *InitFn)(unsigned int);
    typedef void (__cdecl *VoidFn)();
    typedef unsigned int *(__cdecl *ListFn)(int *);
    typedef unsigned short (__cdecl *VendorFn)(unsigned int);
    typedef void *(__cdecl *OpenFn)(unsigned int);
    typedef bool (__cdecl *ConnectedFn)(void *);
    typedef void (__cdecl *CloseFn)(void *);
    typedef short (__cdecl *AxisFn)(void *, int);
    typedef bool (__cdecl *ButtonFn)(void *, int);
    typedef void (__cdecl *FreeFn)(void *);
    typedef bool (__cdecl *PollFn)(void *);
    typedef DWORD (WINAPI *XInputFn)(DWORD, XINPUT_STATE *);

    static HMODULE library = NULL;
    static VoidFn pump = NULL, update = NULL, quit = NULL;
    static ListFn list = NULL;
    static VendorFn vendor = NULL, product = NULL;
    static OpenFn openPad = NULL;
    static ConnectedFn connected = NULL;
    static CloseFn closePad = NULL;
    static AxisFn axis = NULL;
    static ButtonFn button = NULL;
    static FreeFn freeList = NULL;
    static PollFn poll = NULL;
    static void *pad = NULL;
    static XInputFn original = NULL;
    static XINPUT_STATE state = {0};
    static bool available = false;
    static DWORD lastScan = 0;
    static DWORD sequence = 0;

    static DWORD WINAPI ReadState(DWORD index, XINPUT_STATE *result)
    {
        if (!result || !original) return ERROR_DEVICE_NOT_CONNECTED;
        DWORD nativeResult = original(index, result);
        if (nativeResult == ERROR_SUCCESS || index != 0 || !available) return nativeResult;
        *result = state;
        return ERROR_SUCCESS;
    }

    static bool InstallHook()
    {
        BYTE *base = reinterpret_cast<BYTE *>(GetModuleHandleW(NULL));
        IMAGE_DOS_HEADER *dos = reinterpret_cast<IMAGE_DOS_HEADER *>(base);
        if (!base || dos->e_magic != IMAGE_DOS_SIGNATURE) return false;
        IMAGE_NT_HEADERS *nt = reinterpret_cast<IMAGE_NT_HEADERS *>(base + dos->e_lfanew);
        if (nt->Signature != IMAGE_NT_SIGNATURE) return false;
        DWORD imports = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
        if (!imports) return false;
        IMAGE_IMPORT_DESCRIPTOR *desc = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR *>(base + imports);
        for (; desc->Name; ++desc)
        {
            const char *name = reinterpret_cast<const char *>(base + desc->Name);
            if (_strnicmp(name, "xinput", 6) != 0 || !desc->OriginalFirstThunk) continue;
            IMAGE_THUNK_DATA *names = reinterpret_cast<IMAGE_THUNK_DATA *>(base + desc->OriginalFirstThunk);
            IMAGE_THUNK_DATA *slots = reinterpret_cast<IMAGE_THUNK_DATA *>(base + desc->FirstThunk);
            for (; names->u1.AddressOfData; ++names, ++slots)
            {
                if (IMAGE_SNAP_BY_ORDINAL(names->u1.Ordinal)) continue;
                IMAGE_IMPORT_BY_NAME *item = reinterpret_cast<IMAGE_IMPORT_BY_NAME *>(base + names->u1.AddressOfData);
                if (strcmp(reinterpret_cast<char *>(item->Name), "XInputGetState") != 0) continue;
                DWORD oldProtection;
                if (!VirtualProtect(&slots->u1.Function, sizeof(slots->u1.Function), PAGE_READWRITE, &oldProtection)) return false;
                original = reinterpret_cast<XInputFn>(slots->u1.Function);
                slots->u1.Function = reinterpret_cast<ULONG_PTR>(&ReadState);
                DWORD ignored;
                VirtualProtect(&slots->u1.Function, sizeof(slots->u1.Function), oldProtection, &ignored);
                return true;
            }
        }
        return false;
    }

    static bool Initialize()
    {
        library = LoadLibraryW(L"Windows64\\SDL3\\SDL3.dll");
        if (!library) library = LoadLibraryW(L"SDL3.dll");
        if (!library) return false;
#define PC_SDL_LOAD(variable, type, name) variable = reinterpret_cast<type>(GetProcAddress(library, name)); if (!variable) return false
        InitFn init = NULL;
        PC_SDL_LOAD(init, InitFn, "SDL_Init");
        PC_SDL_LOAD(pump, VoidFn, "SDL_PumpEvents");
        PC_SDL_LOAD(poll, PollFn, "SDL_PollEvent");
        PC_SDL_LOAD(update, VoidFn, "SDL_UpdateGamepads");
        PC_SDL_LOAD(quit, VoidFn, "SDL_Quit");
        PC_SDL_LOAD(list, ListFn, "SDL_GetGamepads");
        PC_SDL_LOAD(vendor, VendorFn, "SDL_GetGamepadVendorForID");
        PC_SDL_LOAD(product, VendorFn, "SDL_GetGamepadProductForID");
        PC_SDL_LOAD(openPad, OpenFn, "SDL_OpenGamepad");
        PC_SDL_LOAD(connected, ConnectedFn, "SDL_GamepadConnected");
        PC_SDL_LOAD(closePad, CloseFn, "SDL_CloseGamepad");
        PC_SDL_LOAD(axis, AxisFn, "SDL_GetGamepadAxis");
        PC_SDL_LOAD(button, ButtonFn, "SDL_GetGamepadButton");
        PC_SDL_LOAD(freeList, FreeFn, "SDL_free");
#undef PC_SDL_LOAD
        if (!init(0x00002000u)) return false; // SDL_INIT_GAMEPAD
        return InstallHook();
    }

    static SHORT FlipAxis(SHORT value) { return value == -32768 ? 32767 : static_cast<SHORT>(-value); }

    static void Tick()
    {
        if (!library || !original) return;
        pump();
        union { unsigned __int64 alignment; unsigned char bytes[128]; } event;
        while (poll(&event)) {} // Drain SDL events; the game uses Win32 events.
        update();
        if (pad && !connected(pad)) { closePad(pad); pad = NULL; available = false; }
        DWORD now = GetTickCount();
        if (!pad && (!lastScan || now - lastScan >= 1000))
        {
            lastScan = now;
            int count = 0;
            unsigned int *ids = list(&count);
            for (int i = 0; ids && i < count; ++i)
            {
                if (vendor(ids[i]) != 0x054c) continue;
                unsigned short id = product(ids[i]);
                if (id != 0x0268 && id != 0x05c4 && id != 0x09cc && id != 0x0ce6 && id != 0x0df2) continue;
                pad = openPad(ids[i]);
                if (pad) break;
            }
            if (ids) freeList(ids);
        }
        if (!pad) { available = false; return; }
        XINPUT_STATE next = {0};
        const WORD buttons[] = {XINPUT_GAMEPAD_A, XINPUT_GAMEPAD_B, XINPUT_GAMEPAD_X, XINPUT_GAMEPAD_Y,
            XINPUT_GAMEPAD_BACK, 0, XINPUT_GAMEPAD_START, XINPUT_GAMEPAD_LEFT_THUMB, XINPUT_GAMEPAD_RIGHT_THUMB,
            XINPUT_GAMEPAD_LEFT_SHOULDER, XINPUT_GAMEPAD_RIGHT_SHOULDER, XINPUT_GAMEPAD_DPAD_UP,
            XINPUT_GAMEPAD_DPAD_DOWN, XINPUT_GAMEPAD_DPAD_LEFT, XINPUT_GAMEPAD_DPAD_RIGHT};
        for (int i = 0; i < 15; ++i) if (buttons[i] && button(pad, i)) next.Gamepad.wButtons |= buttons[i];
        next.Gamepad.sThumbLX = axis(pad, 0);
        next.Gamepad.sThumbLY = FlipAxis(axis(pad, 1));
        next.Gamepad.sThumbRX = axis(pad, 2);
        next.Gamepad.sThumbRY = FlipAxis(axis(pad, 3));
        int lt = axis(pad, 4), rt = axis(pad, 5);
        next.Gamepad.bLeftTrigger = static_cast<BYTE>(lt > 0 ? lt * 255 / 32767 : 0);
        next.Gamepad.bRightTrigger = static_cast<BYTE>(rt > 0 ? rt * 255 / 32767 : 0);
        if (memcmp(&next.Gamepad, &state.Gamepad, sizeof(next.Gamepad))) ++sequence;
        next.dwPacketNumber = sequence;
        state = next;
        available = true;
    }

    static bool IsAvailable() { return available; }

    static void Shutdown()
    {
        available = false;
        if (pad && closePad) { closePad(pad); pad = NULL; }
        if (quit) quit();
        // The import hook remains valid until process termination.
    }
}
