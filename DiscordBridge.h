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

BOOL CALLBACK EnumWindowsProcMy(HWND hwnd, LPARAM lParam);
#define GAMEPROCESSOFFLINE EnumWindows(EnumWindowsProcMy, state.cont.getGameProcess())

// Discord Activity Updates
class MyActivity
{
public:
	void generatePresence();
	void joinParty(const char* pw);
	void createParty();
	void updateParty();
};

// Continuum handle code
class Continuum
{
public:
	void startGameClient();
	DWORD getGameProcess();
	bool isSteamUser();
};

struct DiscordState 
{
	Continuum cont;
	MyActivity activity;
	discord::Activity core_activity{};
	discord::User selfusr;
	discord::User requester;
	discord::User inviter;
	std::unique_ptr<discord::Core> core;
};

extern DiscordState state;

// Misc Helper Functions
bool CMPSTART(const char *control, const char *constant);
std::string createHash(int len);

#endif DISCORDBRIDGE_H