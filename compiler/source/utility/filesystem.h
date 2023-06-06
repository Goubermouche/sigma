#pragma once
#include "compiler/diagnostics/error.h"
#include "utility/macros.h"

namespace channel::detail {
	/**
	 * \brief Reads the contents of the specified \a source \a file and, if everything is correct, returns them in the \a out parameter. If there has been an issue when reading the file nothing is read and the function outputs False.
	 * \param filepath Source file path
	 * \return Expected - contents of the file, or, in the case of an erroneous state, an error message.
	 */
	std::expected<std::string, error_msg> read_file(
		const std::string& filepath
	);

	/**
	 * \brief Attempts to delete the given file.
	 * \param filepath Filepath of the file to delete
	 * \return True if the operations succeeded, otherwise false.
	 */
	bool delete_file(
		const std::string& filepath
	);

	/**
	 * \brief Extracts the directory from the given \a filepath.
	 * \param filepath Filepath to extract the directory from
	 * \return New filepath pointing to the extracted directory.
	 */
	std::string extract_directory_from_filepath(
		const std::string& filepath
	);

	/**
	 * \brief Extracts the file name from the given \a filepath.
	 * \param filepath Filepath to extract the filename from
	 * \return String containing the extracted filename.
	 */
	std::string extract_filename_from_filepath(
		const std::string& filepath
	);

	/**
	 * \brief Checks if the given \a filepath points to a valid file.
	 * \param filepath Filepath to check
	 * \return True if the given filepath points to a file, otherwise false.
	 */
	bool is_file(
		const std::string& filepath
	);

	/**
	 * \brief Checks if the given \a filepath points to a valid directory.
	 * \param filepath Filepath to check
	 * \return True if the given filepath points to a directory, otherwise false.
	 */
	bool is_directory(
		const std::string& filepath
	);

	/**
	 * \brief Extracts the file extension from the given \a filepath (including the '.' character).
	 * \param filepath Filepath to extract the extension from
	 * \return String containing the extracted file extension.
	 */
	std::string extract_extension_from_filepath(
		const std::string& filepath
	);
}
