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
	void startGameClient();
	DWORD getGameProcess();
	bool isSteamUser();
	bool inGame();
	int gameWindow();
	int ship = 2;
	bool inMenu = false;
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

#endif DISCORDBRIDGE_H