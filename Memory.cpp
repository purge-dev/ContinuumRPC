#include "Memory.h"

#ifdef UNICODE
#undef UNICODE
#endif

#include <TlHelp32.h>

namespace memory {

    Memory::Memory(DWORD pid) : pid_(pid), module_base_(0), module_base_menu_(0) {
        handle_ = OpenProcess(PROCESS_VM_READ, false, pid);
        module_base_ = GetModuleBase("Continuum.exe");
        module_base_menu_ = GetModuleBase("menu040.dll");
        FetchPlayer();
    }

    void Memory::Update() {
        FetchPlayer();
    }

    void Memory::FetchPlayer() {

        const std::size_t kNameOffset = 0x6D;
        const std::size_t kShipOffset = 0x5C;
        const std::size_t kFreqOffset = 0x58;
        const std::size_t kIdOffset = 0x18;

        std::size_t game_addr_ = ReadU32(handle_, module_base_ + 0xC1AFC);

        std::size_t base_addr = game_addr_ + 0x127EC;
        std::size_t players_addr = base_addr + 0x884;
        std::size_t count_addr = base_addr + 0x1884;

        std::size_t count = ReadU32(handle_, count_addr) & 0xFFFF;

        for (std::size_t i = 0; i < count; ++i) {

            std::size_t player_addr = ReadU32(handle_, players_addr + (i * 4));

            if (!player_addr) continue;

            std::string name = ReadString(handle_, player_addr + kNameOffset, 23);

            if (name == GetName()) {
                player_.name = name;

                player_.id = static_cast<uint16_t>(ReadU32(handle_, player_addr + kIdOffset));
                player_.ship = static_cast<uint16_t>(ReadU32(handle_, player_addr + kShipOffset));
                player_.freq = static_cast<uint16_t>(ReadU32(handle_, player_addr + kFreqOffset));
            }
        }
    }

    std::size_t Memory::GetModuleBase(const char* module_name) {

        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid_);
        MODULEENTRY32 me = { 0 };

        me.dwSize = sizeof(me);

        if (hSnapshot == INVALID_HANDLE_VALUE) {
            return 0;
        }

        std::size_t module_base = 0;

        BOOL bModule = Module32First(hSnapshot, &me);

        while (bModule) {
            if (strcmp(module_name, me.szModule) == 0) {
                module_base = reinterpret_cast<std::size_t>(me.modBaseAddr);
                break;
            }

            bModule = Module32Next(hSnapshot, &me);
        }

        CloseHandle(hSnapshot);

        return module_base;
    }

    std::string Memory::GetName() {
        const std::size_t ProfileStructSize = 2860;

        uint16_t profile_index = ReadU32(handle_, module_base_menu_ + 0x47FA0) & 0xFFFF;
        std::size_t addr = ReadU32(handle_, module_base_menu_ + 0x47A38) + 0x15;

        if (addr == 0) {
            return "";
        }

        addr += profile_index * ProfileStructSize;

        std::string name = ReadString(handle_, addr, 23);

        // remove "^" that gets placed on names when biller is down
        if (!name.empty() && name[0] == '^') {
            name.erase(0, 1);
        }

        name = name.substr(0, strlen(name.c_str()));

        return name;
    }

    uint32_t Memory::ReadU32(HANDLE handle, std::size_t address) {
        uint32_t value = 0;
        SIZE_T num_read;

        if (ReadProcessMemory(handle, (LPVOID)address, &value, sizeof(uint32_t), &num_read)) {
            return value;
        }

        return 0;
    }

    std::string Memory::ReadString(HANDLE handle, std::size_t address, std::size_t len) {
        std::string value;
        SIZE_T read;

        value.resize(len);

        if (ReadProcessMemory(handle, (LPVOID)address, &value[0], len, &read)) {
            return value;
        }

        return "";
    }

} // namespace memory
