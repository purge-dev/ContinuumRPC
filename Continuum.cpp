#include "DiscordBridge.h"

void Continuum::startGameClient()
{
	STARTUPINFO si;
	PROCESS_INFORMATION processInfo;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&processInfo, sizeof(processInfo));

	if (CreateProcess(L"Continuum_main.exe", L"cmd", NULL, NULL, TRUE, 0, NULL, NULL, &si, &processInfo))
	{
		WaitForSingleObject(processInfo.hProcess, INFINITE);
		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);
	}
	return;
}

DWORD Continuum::getGameProcess()
{
	PROCESSENTRY32 entry;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	entry.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(snapshot, &entry))
	{
		do
		{
			if (!lstrcmpi(entry.szExeFile, L"Continuum_main.exe"))
			{
				CloseHandle(snapshot);
				return entry.th32ProcessID;
			}
		} while (Process32Next(snapshot, &entry));
	}

	CloseHandle(snapshot);
}

bool Continuum::isSteamUser()
{
	char path[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, path);

	std::string fullpath = path;
	if (fullpath.find("Steam") != fullpath.npos) // simple but works
		return true;
	else
		return false;
}