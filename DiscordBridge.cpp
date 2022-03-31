#include "DiscordBridge.h"
#include "Memory.h"

discord::Core* core{};
DiscordState state{};
std::unique_ptr<memory::Memory> gMemory;

// Global Variables:
#define MAX_LOADSTRING 100
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// Continuum related Handle Functions
HWND g_HWND = NULL;
HHOOK hHook;

BOOL CALLBACK EnumWindowsProcMy(HWND hwnd, LPARAM lParam)
{
	DWORD lpdwProcessId;
	GetWindowThreadProcessId(hwnd, &lpdwProcessId);
	if (lpdwProcessId == lParam)
	{
		g_HWND = hwnd;
		return FALSE;
	}
	return TRUE;
}

// Misc Helper Functions
bool CMPSTART(const char *control, const char *constant) // ripped from MERV
{
	char c;

	while (c = *control)
	{
		if (*constant != c)
			return false;

		++control;
		++constant;
	}

	return true;
}

std::string multiByteString(const std::wstring& wstr) // unicode to multibyte string
{
    if (wstr.empty()) return std::string();
    int size = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string resStr(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &resStr[0], size, NULL, NULL);
    return resStr;
}

// Keyboard hook to detect ship changes (unreliable since rate limits must be obeyed)
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    KBDLLHOOKSTRUCT* pKbdLLHookStruct = (KBDLLHOOKSTRUCT*)lParam;
    if (state.cont.inGame())
    {
        if (wParam == WM_KEYDOWN)
        {
            switch (pKbdLLHookStruct->vkCode)
            {
            case VK_ESCAPE : 
            {
                if (state.cont.inMenu) // if they are closing the menu without selecting a ship, reset
                    state.cont.inMenu = false;
                else
                    state.cont.inMenu = true;
                break;
            }
			case 0x31:
			{
				if (state.cont.inMenu)
				{
					state.cont.ship = 0;
				}
                break;
			}
            case 0x32:
            {
                if (state.cont.inMenu)
                {
                    state.cont.ship = 1;
                }
                break;
            }
            case 0x33:
            {
                if (state.cont.inMenu)
                {
                    state.cont.ship = 2;
                }
                break;
            }
            case 0x34:
            {
                if (state.cont.inMenu)
                {
                    state.cont.ship = 3;
                }
                break;
            }
            case 0x35:
            {
                if (state.cont.inMenu)
                {
                    state.cont.ship = 4;
                }
                break;
            }
            case 0x36:
            {
                if (state.cont.inMenu)
                {
                    state.cont.ship = 5;
                }
                break;
            }
            case 0x37:
            {
                if (state.cont.inMenu)
                {
                    state.cont.ship = 6;
                }
                break;
            }
            case 0x38:
            {
                if (state.cont.inMenu)
                {
                    state.cont.ship = 7;
                }
                break;
            }
            case 0x53:
            {
                if (state.cont.inMenu)
                {
                    state.cont.ship = 8;
                }
                break;
            }
			} 
        }
    }
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

// Discord Core Initializer 
void DiscordInit()
{
	char install_dir[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, install_dir);
	sprintf_s(install_dir, "%s\\%s", install_dir, "Continuum.exe"); 

    if (discord::Core::Create(611578814073405465, DiscordCreateFlags_NoRequireDiscord, &core) == discord::Result::Ok)
        state.core.reset(core);

	if (!state.core) // Discord failed to instantiate
        state.core->~Core();

    // Discord's game launching command 
    if (state.cont.isSteamUser())
        state.core->ActivityManager().RegisterSteam(352700);        // this will make it appear in Library as Steam game and launch via Steam
    else
        state.core->ActivityManager().RegisterCommand(install_dir); // mainly useful for Join/Spectate activities that are unused here

	// Logger - uncomment when testing
	/*
	state.core->SetLogHook(discord::LogLevel::Error, [](discord::LogLevel level, const char* message){
		MessageBoxA(NULL, message, "Error", MB_ICONERROR);
	});
	*/

	// Relevant Events -> these are received by running RunCallbacks() very often
        /* OnCurrentUserUpdate
    *  Fires when the User struct of the currently connected user changes. They may have changed their avatar, username, or something else. */
    state.core->UserManager().OnCurrentUserUpdate.Connect([]() {
        state.core->UserManager().GetCurrentUser(&state.selfusr);
        });

	/* OnActivityJoin 
	*  Fires when a user accepts a game chat invite or receives confirmation from Asking to Join. */
/*	state.core->ActivityManager().OnActivityJoin.Connect([](const char* secret) { 
		state.activity.joinParty(secret);
	});
	/* OnActivityJoinRequest
	*  Fires when a user asks to join the current user's game. */
/*	state.core->ActivityManager().OnActivityJoinRequest.Connect([](discord::User const& user) {
		state.requester = user;
		state.activity.updateParty();
	});
	/* OnActivityInvite
	*  Fires when the user receives a join or spectate invite.*/
/*	state.core->ActivityManager().OnActivityInvite.Connect([](discord::ActivityActionType, discord::User const& user, discord::Activity const&) {
		state.inviter = user;
	});

	/* OnToggle
	*  Fires when the overlay is locked or unlocked (a.k.a.opened or closed) 
	state.core->OverlayManager().OnToggle.Connect([](bool locked) {       // (needs a handle to THIS process for hook -> ?injection necessary) 
		state.overlayLocked = locked;
	});
	*/
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    DWORD pid = 0;

	// Start Continuum_main.exe
    if (state.cont.startGameClient()) {
        pid = state.cont.GetPid();
        //ENABLE THIS FOR MEMORY READING
        //gMemory = std::make_unique<memory::Memory>(pid);
    }

    // Entry point for SDK-Client interface
	DiscordInit();

	// Begin RPC
	std::thread([=]() {
		std::this_thread::sleep_for(std::chrono::seconds(6)); // wait for SS to load
        if (GAMEPROCESSOFFLINE == 0) // game process is available
        {
            state.activity.generatePresence();
            //ENABLE THESE FOR MEMORY READING
            //gMemory->Update();
            //state.cont.ship = gMemory->GetPlayer().ship;
            state.cont.ship = atoi(multiByteString(state.cont.getRegValue(L"Ship")).c_str()); // sets ship status as last played ship
        }
		else                      // exit out if it hasn't started -> ?remove because of slow PCs
		{
			state.core->~Core();
			exit(0);
		}}).detach();

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DISCORDBRIDGE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DISCORDBRIDGE));
    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, 0);

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {   
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
			state.core->RunCallbacks();
        }
    }

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DISCORDBRIDGE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DISCORDBRIDGE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
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
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

 //  ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0); 
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
