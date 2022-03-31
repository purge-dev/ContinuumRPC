#ifndef DISCORDBRIDGE_H
#define DISCORDBRIDGE_H
#include "discordsdk\discord.h"
#include "resource.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#include <windows.h>
#include <SDKDDKVer.h>

// C RunTime Header Files
#include <iostream>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <chrono>
#include <thread>
#include <string>
#include <tlhelp32.h>
/*
#include <Shlobj.h>
#include <shlwapi.h>
*/

BOOL CALLBACK EnumWindowsProcMy(HWND hwnd, LPARAM lParam);
#define GAMEPROCESSOFFLINE EnumWindows(EnumWindowsProcMy, state.cont.getGameProcess())

// Discord Activity Updates
class MyActivity
{
public:
	void generatePresence();
	void setMainRPC(const char* largeImage, const char* largeImageText, discord::ActivityType type);
	void setSubRPC(const char* stateText, const char* smallImage, const char* smallImageText, const char* details, discord::Timestamp timer);
	/*
	void joinParty(const char* pw);
	void createParty();
	void updateParty();
	*/
};

// Continuum handle code
class Continuum
{
public:
	bool startGameClient();
	DWORD getGameProcess();
	bool isSteamUser();
	bool inGame();
	int gameWindow();
	std::wstring getRegValue(std::wstring val);
	// allow public read access to pid via a return function 
	DWORD& GetPid() { return pid_; }

//	uintptr_t GetModuleBaseAddress(DWORD dwProcID);
//	uint8_t GetShip();
	HANDLE gHandle;
	int ship;
	bool inMenu = false;
private:
	// keep this private so only the Continuum class members can modify it
	DWORD pid_;
};

struct DiscordState 
{
	Continuum cont;
	MyActivity activity;
	discord::Activity core_activity{};
	discord::User selfusr;
	std::unique_ptr<discord::Core> core;
};

extern DiscordState state;

// Misc Helper Functions
bool CMPSTART(const char *control, const char *constant);
std::string multiByteString(const std::wstring& wstr);
//bool DiscordInstalled();

#endif DISCORDBRIDGE_H