#pragma once
#include "macros.h"

namespace utility {
	/**
	 * \brief Basic shell abstraction, used for invoking commands on the local system.
	 */
	struct shell {
		/**
		 * \brief Runs the specified \b command in the local shell.
		 * \param command Command to run
		 * \return 0 if all operations succeeded, non-zero otherwise.
		 */
		static auto execute(const std::string& command) -> i32 {
#ifdef SYSTEM_WINDOWS
			const std::string windows_command = "cmd /c " + command;
			return system(windows_command.c_str());
#elif defined SYSTEM_LINUX
			i32 status = system(command.c_str());
			if (WIFEXITED(status)) {
				return WEXITSTATUS(status);
			}
			else {
				// abnormal termination
				return -1;
			}
#else
			PANIC("unsupported platform detected");
#endif
		}

		/**
		 * \brief Opens the specified \b link in the local browser.
		 * \param link Link to open
		 * \return 0 if all operations succeeded, non-zero otherwise.
		 */
		static auto open_link(const std::string& link) -> i32 {
			std::string command;
#ifdef SYSTEM_WINDOWS
			command = "start " + link;
#elif defined SYSTEM_LINUX
			command = "xdg-open " + link;
#else
			PANIC("unsupported platform detected");
#endif

			return execute(command);
		}
	};
} // namespace utility
