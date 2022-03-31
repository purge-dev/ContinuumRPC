#include "DiscordBridge.h"

std::string ReadString(HANDLE handle, size_t address, size_t len)
{
	std::string value;
	SIZE_T read;

	value.resize(len);

	if (ReadProcessMemory(handle, (LPVOID)address, &value[0], len, &read)) {
		return value;
	}

	return "";
}

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
		gHandle = processInfo.hProcess;

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

bool Continuum::inGame()
{
	HWND hwnd;

	if (state.cont.isSteamUser())
		hwnd = FindWindow(0, _T("Subspace Continuum")); // steam users
	else
		if (FindWindow(0, _T("Continuum")) == NULL)
			hwnd = FindWindow(0, _T("Subspace Continuum")); // discord installs
		else
			hwnd = FindWindow(0, _T("Continuum")); // legacy
	if (GetForegroundWindow() == hwnd) // active game window
		return true;
	else
		return false;
}

int Continuum::gameWindow()
{
	char text[32];
	HWND alt = GetForegroundWindow();
	GetWindowTextA(alt, text, 32);

	if ((CMPSTART("Subspace Continuum 0.40 -", text) || (CMPSTART("Continuum 0.40 -", text)))) // chat menu
		return 1;
	else if ((CMPSTART("Subspace Continuum 0.40", text) || (CMPSTART("Continuum 0.40", text)))) // launcher
		return 2;
	else
		if ((FindWindow(0, _T("Continuum")) == NULL) && (FindWindow(0, _T("Subspace Continuum")) == NULL)) // not idling in game but launcher open
			return 4;
		else // true idle
			return 3;
}

std::wstring Continuum::getRegValue(std::wstring val)
{
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Continuum\\State", 0, KEY_READ, &hKey) != ERROR_SUCCESS)
		throw "Registry entry not found!";

	DWORD type, cbData, buff;

	if (RegQueryValueEx(hKey, val.c_str(), NULL, &type, NULL, &cbData) != ERROR_SUCCESS)
		RegCloseKey(hKey);

	std::wstring value(cbData / sizeof(wchar_t), L'\0');
	if (type == REG_DWORD) // ie. ship number
	{
		if (RegQueryValueEx(hKey, val.c_str(), NULL, &type, (LPBYTE)&buff, &cbData) != ERROR_SUCCESS)
			RegCloseKey(hKey);

		return std::to_wstring(buff);
	}
	else if (type == REG_SZ) // ie. zone name/skin
	{
		if (RegQueryValueEx(hKey, val.c_str(), NULL, NULL, reinterpret_cast<LPBYTE>(&value[0]), &cbData) != ERROR_SUCCESS)
			RegCloseKey(hKey);

		size_t firstNull = value.find_first_of(L'\0');
		if (firstNull != std::string::npos)
			value.resize(firstNull);

		return value;
	}
	RegCloseKey(hKey);
}
/*
// TODO: Get player's ship by reading from memory. Unable to get base address for some reason now.
uintptr_t Continuum::GetModuleBaseAddress(DWORD dwProcID)
{
	uintptr_t ModuleBaseAddress = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, dwProcID);
	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 ModuleEntry32;
		ModuleEntry32.dwSize = sizeof(MODULEENTRY32);
		if (Module32First(hSnapshot, &ModuleEntry32))
		{
			do
			{
				if (!_wcsicmp(ModuleEntry32.szModule, L"Continuum_main.exe"))
				{
					ModuleBaseAddress = (uintptr_t)ModuleEntry32.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnapshot, &ModuleEntry32));
		}
		
	}
	CloseHandle(hSnapshot);
	return ModuleBaseAddress;
}
*/