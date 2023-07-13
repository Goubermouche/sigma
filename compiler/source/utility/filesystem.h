#pragma once
#include "diagnostics/error.h"
#include "utility/macros.h"

namespace sigma::detail {
	/**
	 * \brief Reads the contents of the specified \a source \a file and, if everything is correct, returns them in the \a out parameter. If there has been an issue when reading the file nothing is read and the function outputs False.
	 * \param path Source file path
	 * \return Expected - contents of the file, or, in the case of an erroneous state, an error message.
	 */
	outcome::result<std::string> read_file(
		const filepath& path
	);

	/**
	 * \brief Attempts to delete the given file.
	 * \param path Filepath of the file to delete
	 * \return True if the operations succeeded, otherwise false.
	 */
	bool delete_file(
		const filepath& path
	);

	/**
	 * \brief Extracts the directory from the given \a filepath.
	 * \param path Filepath to extract the directory from
	 * \return New filepath pointing to the extracted directory.
	 */
	std::string extract_directory_from_filepath(
		const filepath& path
	);

	/**
	 * \brief Extracts the file name from the given \a filepath.
	 * \param path Filepath to extract the filename from
	 * \return String containing the extracted filename.
	 */
	std::string extract_filename_from_filepath(
		const filepath& path
	);

	/**
	 * \brief Checks if the given \a filepath points to a valid file.
	 * \param path Filepath to check
	 * \return True if the given filepath points to a file, otherwise false.
	 */
	bool is_file(
		const filepath& path
	);

	/**
	 * \brief Checks if the given \a filepath points to a valid directory.
	 * \param path Filepath to check
	 * \return True if the given filepath points to a directory, otherwise false.
	 */
	bool is_directory(
		const filepath& path
	);

	/**
	 * \brief Extracts the file extension from the given \a filepath (including the '.' character).
	 * \param path Filepath to extract the extension from
	 * \return String containing the extracted file extension.
	 */
	std::string extract_extension_from_filepath(
		const filepath& path
	);
}
