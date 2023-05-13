#pragma once
#include "utility/macros.h"

namespace channel::detail {
	/**
	 * \brief Reads the contents of the specified \a source \a file and, if everything is correct, returns them in the \a out parameter. If there has been an issue when reading the file nothing is read and the function outputs False.
	 * \param file_path Source file path
	 * \param out Output file data
	 * \return False when there is an issue when reading the file, otherwise True
	 */
	bool read_file(
		const std::string& file_path,
		std::string& out
	);

	bool delete_file(
		const std::string& file_path
	);

	std::string extract_directory_from_filepath(
		const std::string& filepath
	);

	std::string extract_filename_from_filepath(
		const std::string& filepath
	);

	bool is_file(
		const std::string& filepath
	);

	bool is_directory(
		const std::string& filepath
	);

	std::string extract_extension_from_filepath(
		const std::string& filepath
	);
}