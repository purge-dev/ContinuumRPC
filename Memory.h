#pragma once
#include <windows.h>
#include <cstddef>
#include <string>

namespace memory {


	struct Player {
		uint16_t ship;
		uint16_t freq;
		uint16_t id;
		std::string name;
	};

	class Memory {
	public:
		Memory(DWORD pid);

		~Memory() { if (handle_) CloseHandle(handle_); }

		void Update();

		const Player& GetPlayer() const { return player_; }

	private:
		void FetchPlayer();
		std::size_t GetModuleBase(const char* module_name);
		std::string GetName();
		uint32_t ReadU32(HANDLE handle, std::size_t address);
		std::string ReadString(HANDLE handle, std::size_t address, std::size_t len);

		Player player_;
		HANDLE handle_;
		DWORD pid_;
		std::size_t module_base_;
		std::size_t module_base_menu_;

	};
} // namespace memory
