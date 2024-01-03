#include "shell.h"
#include "utility/macros.h"

namespace utility {
	auto shell::execute(const std::string& command) -> i32 {
#ifdef SYSTEM_WINDOWS
		const std::string windows_command = "cmd /c " + command;
		return system(windows_command.c_str());
#elif SYSTEM_LINUX
		return system(command.c_str());
#else
		PANIC("unsupported platform detected");
#endif
	}

	auto shell::open_link(const std::string& link) -> i32 {
		std::string command;
#ifdef SYSTEM_WINDOWS
		command = "start " + link;
#elif SYSTEM_LINUX
		command = "xdg-open " + link;
#else
		PANIC("unsupported platform detected");
#endif

		return execute(command);
	}
} // namespace utility
