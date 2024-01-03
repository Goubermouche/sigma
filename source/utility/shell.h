#pragma once
#include "utility/macros.h"

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
		static auto execute(const std::string& command) -> i32;

		/**
		 * \brief Opens the specified \b link in the local browser.
		 * \param link Link to open
		 * \return 0 if all operations succeeded, non-zero otherwise.
		 */
		static auto open_link(const std::string& link) -> i32;
	};
} // namespace utility
