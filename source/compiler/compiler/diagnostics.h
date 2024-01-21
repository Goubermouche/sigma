#pragma once
namespace sigma {
	// TODO: add positional information to errors generated by the type checker

	/**
	 * \brief Basic handler for errors generated during compilation, abstracts over utility::error.
	 */
	class error {
	public:
		/**
		 * \brief Predeclared error codes
		 */
		enum class code : u32 {
			// filesystem (1000 - 1999)
			EXPECTED_FILE = 1000,
			FILE_DOES_NOT_EXIST,
			INVALID_FILE_EXTENSION,
			CANNOT_READ_FILE,

			// tokenizer (2000 - 2999)
			INVALID_STRING_TERMINATOR = 2000,
			INVALID_CHAR_TERMINATOR,
			NUMERICAL_LITERAL_FP_WITHOUT_DOT,
			NUMERICAL_LITERAL_UNSIGNED_WITH_DOT,
			NUMERICAL_LITERAL_MORE_THAN_ONE_DOT,

			// parser (3000 - 3999)
			UNEXPECTED_NON_NUMERICAL = 3000,
			UNEXPECTED_TOKEN,
			UNEXPECTED_TOKEN_WITH_EXPECTED,

			// type checker (4000 - 4999)
			UNKNOWN_FUNCTION = 4000,
			NO_CALL_OVERLOAD,
			FUNCTION_ALREADY_DECLARED,
			VOID_RETURN,
			UNEXPECTED_TYPE,
			UNKNOWN_VARIABLE,
			UNKNOWN_VARIABLE_ASSIGN,
			VARIABLE_ALREADY_DECLARED,
			UNKNOWN_NAMESPACE,
			VOID_VARIABLE,
		};

		/**
		 * \brief Emits a new error with positional info using an error code.
		 * \tparam arguments Argument types for the specific \b code
		 * \param code Error code to emit
		 * \param location Handle to the relevant token location
		 * \param args Relevant error arguments (arguments for the specific error::code message)
		 * \return utility::error with the generated message.
		 */
		template<typename... arguments>
		static auto emit(code code, handle<token_location> location, arguments&&... args) -> utility::error {
			const std::string str = std::format(
				"{}:{}:{}: error C{}: {}",
				location->file->get_filename(),
				location->line_index + 1, 
				location->char_index + 1, 
				static_cast<u32>(code), 
				m_errors.find(code)->second
			);

			return utility::error(str, std::forward<arguments>(args)...);
		}

		/**
		 * \brief Emits a new error using an error code.
		 * \tparam arguments Argument types for the specific \b code
		 * \param code Error code to emit
		 * \param args relevant error arguments (arguments for the specific error::code message)
		 * \return utility::error with the generated message.
		 */
		template<typename... arguments>
		static auto emit(code code, arguments&&... args) -> utility::error {
			const std::string str = std::format("error C{}: {}", static_cast<u32>(code), m_errors.find(code)->second);
			return utility::error(str, std::forward<arguments>(args)...);
		}
	private:
		const static inline std::unordered_map<code, std::string> m_errors = {
			// filesystem
			{ code::EXPECTED_FILE,                       "expected a file but got a directory ('{}')"                                        },
			{ code::FILE_DOES_NOT_EXIST,                 "specified file does not exist ('{}')"                                              },
			{ code::INVALID_FILE_EXTENSION,              "specified file has an invalid file extension ('{}' - expected '{}')"               },
			{ code::CANNOT_READ_FILE,                    "unable to read the specified file ('{}')"                                          },

			// tokenizer
			{ code::INVALID_STRING_TERMINATOR,           "invalid string literal terminator detected"                                        },
			{ code::INVALID_CHAR_TERMINATOR,             "invalid char literal terminator detected"                                          },
			{ code::NUMERICAL_LITERAL_FP_WITHOUT_DOT,    "numerical floating-point literal without '.' character detected"                   },
			{ code::NUMERICAL_LITERAL_UNSIGNED_WITH_DOT, "unsigned numerical literal with '.' character detected"                            },
			{ code::NUMERICAL_LITERAL_MORE_THAN_ONE_DOT, "numerical literal with more than one '.' character detected"                       },

			// parser
			{ code::UNEXPECTED_NON_NUMERICAL,            "unexpected non-numerical literal token received ('{}')"                            },
			{ code::UNEXPECTED_TOKEN,                    "unexpected token received ('{}')"                                                  },
			{ code::UNEXPECTED_TOKEN_WITH_EXPECTED,      "unexpected token received (expected '{}', but got '{}' instead)"                   },

			// type checker
			{ code::UNKNOWN_FUNCTION,                    "attempting to call an unknown function ('{}')"                                     },
			{ code::NO_CALL_OVERLOAD,                    "no matching overload for call to function '{}' found - candidates considered:\n{}" },
			{ code::FUNCTION_ALREADY_DECLARED,           "function '{}' has already been declared before"                                    },
			{ code::VOID_RETURN,                         "invalid void return statement (expected 'ret;', but got 'ret {};' instead)"        },
			{ code::UNEXPECTED_TYPE,                     "encountered an unexpected type (expected '{}', but got '{}' instead)"              },
			{ code::UNKNOWN_VARIABLE,                    "unknown variable '{}' referenced"                                                  },
			{ code::UNKNOWN_VARIABLE_ASSIGN,             "attempting to assign to an unknown variable '{}'"                                  },
			{ code::VARIABLE_ALREADY_DECLARED,           "variable '{}' has already been declared before"                                    },
			{ code::UNKNOWN_NAMESPACE,                   "attempting to access an unknown namespace '{}'"                                    },
			{ code::VOID_VARIABLE,                       "cannot declare a variable of type 'void' ('{}')"                                   },
		};
	};

	class warning {
	public:
		enum class code : u32 {
			LITERAL_OVERFLOW = 1000,
			IMPLICIT_EXTENSION_CAST,
			IMPLICIT_TRUNCATION_CAST,
		};

		template<typename... arguments>
		static void emit(code code, handle<token_location> location, arguments&&... args) {
			auto formatted_args = std::make_format_args(std::forward<arguments>(args)...);
			std::string message = std::vformat(m_warnings.find(code)->second, formatted_args);
			utility::console::print(
				"{}:{}:{}: warning C{}: {}\n",
				location->file->get_filename(),
				location->line_index + 1,
				location->char_index + 1,
				static_cast<u32>(code), 
				message);
		}
	private:
		const static inline std::unordered_map<code, std::string> m_warnings = {
			// filesystem
			{ code::LITERAL_OVERFLOW,         "literal overflow detected ('{}' to '{}' for type '{}')" },
			{ code::IMPLICIT_EXTENSION_CAST,  "implicit extending type cast ('{}' to '{}')"            },
			{ code::IMPLICIT_TRUNCATION_CAST, "implicit truncating type cast ('{}' to '{}')"           },
		};
	};
} // namespace sigma
