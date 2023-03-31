#pragma once
#include "macros.h"

namespace channel::detail {
	/**
	 * \brief Reads the contents of the specified \a source \a file and, if everything is correct, returns them in the \a out parameter. If there has been an issue when reading the file nothing is read and the function outputs False.
	 * \param source_file Source file path
	 * \param out Output file data
	 * \return False when there is an issue when reading the file, otherwise True
	 */
	bool read_file(const std::string& source_file, std::string& out) ;
}