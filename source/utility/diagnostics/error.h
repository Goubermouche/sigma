#pragma once
#include "utility/diagnostics/diagnostics.h"

namespace utility {
	enum class error_code {
		// ******************************************************************
		// system
		// ******************************************************************
		unable_to_open_file = 1000,
		unable_to_open_file_reason,
		unable_to_delete_file,
		file_does_not_exist,
		file_expected_but_got_directory,
		directory_expected_but_got_file,
		unable_to_seek_end_of_file,
		unable_to_read_file,
		invalid_file_extension,
		string_to_hexadecimal_failed,
		// ******************************************************************
		// lexer
		// ******************************************************************
		lexer_invalid_dot_at_start = 2000,
		lexer_double_underscore,
		lexer_invalid_number_format_more_than_one_dot,
		lexer_invalid_number_format_unsigned_containing_dot,
		lexer_invalid_number_format_floating_point_without_dot,
		lexer_unterminated_character_literal,
		lexer_unterminated_string_literal,
		lexer_unknown_escape_sequence,
		// ******************************************************************
		// parser
		// ******************************************************************
		parser_unexpected_token = 3000,
		parser_unhandled_token,
		parser_unhandled_non_numerical_token,
		parser_unhandled_non_type_token,
		parser_cannot_apply_unary_to_non_identifier,
		// ******************************************************************
		// codegen
		// ******************************************************************
		function_already_defined = 4000,
		function_cannot_be_found,
		function_argument_count_mismatch,
		variable_cannot_be_found,
		local_variable_already_defined_in_global_scope,
		local_variable_already_defined,
		global_variable_already_defined,
		return_statement_type_mismatch,
		for_conditional_operator_not_bool,
		break_statement_out_of_loop_body,
		cannot_find_main,
		cannot_find_main_with_correct_return_type,
		cannot_declare_numerical_using_pointer,
		cannot_declare_numerical_using_type,
		ir_module_contains_errors,
		global_variable_already_defined_at,
		function_already_defined_at,
		// unary errors (4100)
		post_decrement_type_mismatch = 4100,
		post_increment_type_mismatch,
		pre_decrement_type_mismatch,
		pre_increment_type_mismatch,
		negation_type_mismatch,
		bitwise_not_type_mismatch,
		// binary errors (4200)
		conjunction_type_mismatch = 4200,
		disjunction_type_mismatch,
		bitwise_and_type_mismatch,
		bitwise_or_type_mismatch,
		bitwise_left_shift_type_mismatch,
		bitwise_right_shift_type_mismatch,
		bitwise_xor_type_mismatch,
		// ******************************************************************
		// compiler
		// ******************************************************************
		target_machine_cannot_emit_file = 5000,
		clang_compilation_contains_errors,
		cannot_lookup_target,
		// dependency graph (5100)
		circular_dependency = 5100,
		// settings parser
		unrecognized_argument_value = 5200,
		unrecognized_output_extension = 5201,
	};

	struct error_message : public diagnostic_message {
		error_message(
			error_code code,
			const std::string& message
		);

		virtual void print() override;
	protected:
		error_code m_code;
	};

	struct error_message_range : public error_message {
		error_message_range(
			error_code code,
			const std::string& message,
			const file_range& range
		);

		void print() override;
	protected:
		file_range m_range;
	};

	class error {
	public:
		template<error_code code, typename...argument_types>
		static ptr<error_message> emit(argument_types... args) {
			const auto it = m_error_templates.find(code);
			if (it == m_error_templates.end()) {}

			return std::make_shared<error_message>(code, detail::format(it->second, args...));
		}

		template<error_code code, typename...argument_types>
		static ptr<error_message_range> emit(
			const file_range& range,
			argument_types... args
		) {
			const auto it = m_error_templates.find(code);
			if (it == m_error_templates.end()) {}

			return std::make_shared<error_message_range>(code, detail::format(it->second, args...), range);
		}
	private:
		static inline std::unordered_map<error_code, std::string> m_error_templates = {
			// ******************************************************************
			// system
			// ******************************************************************
			{ error_code::unable_to_open_file,                                    "'{}': unable to open file"                                                                                },
			{ error_code::unable_to_open_file_reason,                             "'{}': unable to open file ('{}')"                                                                                },
			{ error_code::unable_to_delete_file,                                  "'{}': unable to delete file"                                                                              },
			{ error_code::file_does_not_exist,                                    "'{}': file does not exist"                                                                                },
			{ error_code::file_expected_but_got_directory,                        "'{}': directory was not expected (expected a file)"                                                       },
			{ error_code::directory_expected_but_got_file,                        "'{}': file was not expected (expected a directory)"                                                       },
			{ error_code::unable_to_seek_end_of_file,                             "'{}': unable to seek end of file"                                                                         },
			{ error_code::unable_to_read_file,                                    "'{}': unable to read file"                                                                                },
			{ error_code::invalid_file_extension,                                 "'{}': invalid file extension (expected '{}')"                                                             },
			{ error_code::string_to_hexadecimal_failed,                           "'{}': string to hexadecimal conversion failed (the expected item was not found or a read error occurred)" },
			// ******************************************************************
			// lexer
			// ******************************************************************
			{ error_code::lexer_invalid_dot_at_start,                             "'lexer': invalid '.' character detected at token start"                                                   },
			{ error_code::lexer_double_underscore,                                "'lexer': two '_' characters immediately one after another are not allowed"                                },
			{ error_code::lexer_invalid_number_format_more_than_one_dot,      "'lexer': invalid number format - cannot declare a number with more that one '.' character"                    },
			{ error_code::lexer_invalid_number_format_unsigned_containing_dot,    "'lexer': invalid number format - cannot declare an unsigned number containing '.' characters"             },
			{ error_code::lexer_invalid_number_format_floating_point_without_dot, "'lexer': invalid number format - floating point number must contain a '.' character"                      },
			{ error_code::lexer_unterminated_character_literal,                   "'lexer': invalid unterminated character literal detected"                                                 }, 
			{ error_code::lexer_unterminated_string_literal,                      "'lexer': invalid unterminated string literal detected"                                                    },
			{ error_code::lexer_unknown_escape_sequence,                          "'lexer': unrecognized escape sequence '\\{}' detected"                                                    },
			// ******************************************************************
			// parser
			// ******************************************************************
			{ error_code::parser_unexpected_token,                                "'parser': unexpected token received (expected '{}', but received '{}' instead)"                           },
			{ error_code::parser_unhandled_token,                                 "'parser': unhandled token received ('{}')"                                                                },
			{ error_code::parser_unhandled_non_numerical_token,                   "'parser': unhandled non-numerical token received ('{}')"                                                  },
			{ error_code::parser_unhandled_non_type_token,                        "'parser': unexpected non-type token received ('{}')"                                                      },
			{ error_code::parser_cannot_apply_unary_to_non_identifier,            "'parser': cannot apply a unary operator to a non-identifier value"                                        },
			// ******************************************************************
			// codegen
			// ******************************************************************
			{ error_code::function_already_defined,                               "'{}': function has already been defined before"                                                           },
			{ error_code::function_cannot_be_found,                               "'{}': function cannot be found"                                                                           },
			{ error_code::function_argument_count_mismatch,                       "'{}': function argument count mismatch"                                                                   },
			{ error_code::variable_cannot_be_found,                               "'{}': variable cannot be found"                                                                           },
			{ error_code::local_variable_already_defined_in_global_scope,         "'{}': local variable has already been defined in the global scope"                                        },
			{ error_code::local_variable_already_defined,                         "'{}': local variable has already been defined before"                                                     },
			{ error_code::global_variable_already_defined,                        "'{}': global variable has already been defined before"                                                    },
			{ error_code::return_statement_type_mismatch,                         "'{}': return statement type '{}' does not match the expected function return type '{}'"                   },
			{ error_code::for_conditional_operator_not_bool,                      "for conditional operator has to be of type 'bool' (received '{}' instead)"                                },
			{ error_code::break_statement_out_of_loop_body,                       "invalid break statement detected outside a valid loop body"                                               },
			{ error_code::cannot_find_main,                                       "unable to locate the main entry point"                                                                    },
			{ error_code::cannot_find_main_with_correct_return_type,              "unable to declare a main entry point with the return type of '{}' (expecting 'i32')"                      },
			{ error_code::cannot_declare_numerical_using_pointer,                 "unable to declare a numerical literal using a pointer type"                                               },
			{ error_code::cannot_declare_numerical_using_type,                    "unable to declare a numerical literal using the given type '{}'"                                          },
			{ error_code::ir_module_contains_errors,                              "IR module contains errors"                                                                                },
			{ error_code::global_variable_already_defined,                        "'{}': global variable has already been defined before at '{}'"                                            },
			{ error_code::function_already_defined_at,                            "'{}': function has already been defined before at '{}'"                                                   },
			// unary errors (4100)
			{ error_code::post_decrement_type_mismatch,                           "'post decrement': operator expected a numerical value, but got '{}' instead"                              },
			{ error_code::post_increment_type_mismatch,                           "'post increment': operator expected a numerical value, but got '{}' instead"                              },
			{ error_code::pre_decrement_type_mismatch,                            "'pre decrement': operator expected a numerical value, but got '{}' instead"                               },
			{ error_code::pre_increment_type_mismatch,                            "'pre increment': operator expected a numerical value, but got '{}' instead"                               },
			{ error_code::negation_type_mismatch,                                 "'negation': operator expected a scalar value, but got '{}' instead"                                       },
			{ error_code::bitwise_not_type_mismatch,                              "'bitwise not': operation expects an integral, but received '{}' instead"                                  },
			// binary errors (4200)
			{ error_code::conjunction_type_mismatch,                              "'conjunction': operation expects 2 booleans, but received '{}' and '{}' instead"                          },
			{ error_code::disjunction_type_mismatch,                              "'disjunction': operation expects 2 booleans, but received '{}' and '{}' instead"                          },
			{ error_code::bitwise_and_type_mismatch,                              "'bitwise and': operation expects 2 integrals, but received '{}' and '{}' instead"                         },
			{ error_code::bitwise_or_type_mismatch,                               "'bitwise or': operation expects 2 integrals, but received '{}' and '{}' instead"                          },
			{ error_code::bitwise_left_shift_type_mismatch,                       "'bitwise left shift': operation expects 2 integrals, but received '{}' and '{}' instead"                  },
			{ error_code::bitwise_right_shift_type_mismatch,                      "'bitwise right shift': operation expects 2 integrals, but received '{}' and '{}' instead"                 },
			{ error_code::bitwise_xor_type_mismatch,                              "'bitwise xor': operation expects 2 integrals, but received '{}' and '{}' instead"                         },
			// ******************************************************************
			// compiler
			// ******************************************************************
			{ error_code::target_machine_cannot_emit_file,                        "the target machine cannot emit a file of this type"                                                       },
			{ error_code::clang_compilation_contains_errors,                      "clang compilation contains errors"                                                                        },
			{ error_code::cannot_lookup_target,                                   "unable to lookup target '{}' ('{}')"                                                                      },
			// dependency graph (5100)
			{ error_code::circular_dependency,                                    "circular dependency detected"                                                                             },
			// settings parser
			{ error_code::unrecognized_argument_value,                            "unrecognized value '{}' received for argument '{}'"                                                       },
			{ error_code::unrecognized_output_extension,                          "unrecognized filepath received for output argument"                                                       },
		};
	}; 

	namespace outcome {
		/**
		 * \brief Base \a failure class, can be emitted with an \a error.
		 */
		class failure {
		public:
			failure() = delete;

			/**
			 * \brief Creates a new failure case containing the given \a error.
			 * \param error Error to use as the reason for failure
			 */
			failure(
				ptr<error_message> error
			) : m_error(error) {}

			/**
			 * \brief Returns the contained error.
			 */
			ptr<error_message> get_error() const {
				return m_error;
			}
		private:
			ptr<error_message> m_error;
		};

		/**
		 * \brief Base success class, used for handling success cases with no success return types.
		 */
		class success {};

		/**
		 * \brief Base result class, contains information about the outcome of an operation.
		 * \tparam type Type of the successful outcome
		 */
		template<typename type>
		class result {
		public:
			/**
			 * \brief Move constructor.
			 */
			constexpr result(
				result&& other
			) noexcept {
				m_value = other.m_value;
			}

			/**
			 * \brief Constructs a result from a failure.
			 */
			constexpr result(
				failure&& failure
			) : m_value(std::unexpected(failure.get_error())) {}

			/**
			 * \brief Constructs a result from a success.
			 */
			constexpr result(
				success&& success
			) : m_value() {}

			/**
			 * \brief Constructs a result from a given value.
			 */
			template<typename current_type>
			constexpr result(
				current_type&& value
			) : m_value(std::forward<current_type>(value)) {}

			/**
			 * \brief Checks if the result contains an error.
			 */
			bool has_error() const {
				return !m_value.has_value();
			}

			/**
			 * \brief Checks if the result contains a value.
			 */
			bool has_value() const {
				return m_value.has_value();
			}

			/**
			 * \brief Returns the encapsulated value.
			 */
			const type& get_value() const {
				return m_value.value();
			}

			/**
			 * \brief Returns the encapsulated error.
			 */
			const ptr<error_message> get_error() const {
				return m_value.error();
			}
		private:
			std::expected<type, ptr<error_message>> m_value;
		};

		/**
		 * \brief Specialization of the \a result class for void type.
		 */
		template<>
		class result<void> {
		public:
			constexpr result(
				result&& other
			) noexcept {
				m_value = other.m_value;
			}

			constexpr result(
				failure&& failure
			) : m_value(std::unexpected(failure.get_error())) {}

			constexpr result(
				success&& success
			) : m_value() {
				SUPPRESS_C4100(success);
			}

			bool has_error() const {
				return !m_value.has_value();
			}

			bool has_value() const {
				return m_value.has_value();
			}

			ptr<error_message> get_error() const {
				return m_value.error();
			}
		private:
			std::expected<void, ptr<error_message>> m_value;
		};
	}
}
