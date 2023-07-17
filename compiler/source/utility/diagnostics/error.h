#pragma once
#include "utility/diagnostics/diagnostic.h"

namespace sigma {
	class error_message {
	public:
		error_message(
			u64 error_code,
			std::string message
		) : m_error_code(error_code),
			m_message(std::move(message)) {}

		friend console& operator<<(
			console& out, 
			error_message& message
		) {
			message.print();
			return out;
		}

	protected:
		virtual void print() {
			console::out
				<< color::red
				<< "error "
				<< m_error_code
				<< ": "
				<< m_message
				<< "\n"
				<< color::white;
		}
	protected:
		u64 m_error_code;
		std::string m_message;
	};

	class error_position : public error_message {
	public:
		error_position(
			u64 error_code,
			std::string message,
			const file_position& position
		) : error_message(error_code, std::move(message)),
			m_position(position) {}

	protected:
		void print() override {
			console::out
				<< m_position
				<< ":   "
				<< color::red
				<< "error "
				<< m_error_code
				<< ": "
				<< m_message
				<< "\n"
				<< color::white;
		}
	private:
		file_position m_position;
	};

	/**
	 * \brief Base error class
	 */
	class error {
		template<u64 error_code>
		static inline const char* error_template{};
	public:

		template<u64 error_code, typename... argument_types>
		static std::shared_ptr<error_position> emit(
			const file_position& position,
			argument_types&&... arguments
		) {
			return std::make_shared<error_position>(error_code,
				std::vformat(
					error_template<error_code>,
					std::make_format_args(std::forward<argument_types>(arguments)...)
				),
				position
			);
		}

		/**
		 * \brief Emits a new error using the given \a error code.
		 * \tparam error_code Error code of the error to emit
		 * \tparam argument_types Argument list types
		 * \param arguments Argument list that will be passed to the error template
		 * \return \a Error with the given error code and generated error message.
		 */
		template<u64 error_code, typename... argument_types>
		static std::shared_ptr<error_message> emit(
			argument_types&&... arguments
		) {
			return std::make_shared<error_message>(error_code,
				std::vformat(
					error_template<error_code>,
					std::make_format_args(std::forward<argument_types>(arguments)...)
				)
			);
		}

	};

	// *********************************************************************************************************************
	// system
	// *********************************************************************************************************************
	template<> const char* error::error_template<1000> = "'{}': unable to open file";
	template<> const char* error::error_template<1001> = "'{}': unable to delete file";
	template<> const char* error::error_template<1002> = "'{}': file does not exist";
	template<> const char* error::error_template<1003> = "'{}': directory was not expected (expected a file)";
	template<> const char* error::error_template<1004> = "'{}': file was not expected (expected a directory)";
	template<> const char* error::error_template<1005> = "'{}': unable to seek end of file";
	template<> const char* error::error_template<1006> = "'{}': unable to read file";
	template<> const char* error::error_template<1007> = "'{}': invalid file extension (expected '{}')";

	// *********************************************************************************************************************
	// lexer
	// *********************************************************************************************************************
	template<> const char* error::error_template<2000> = "'lexer': invalid '.' character detected at token start";
	template<> const char* error::error_template<2001> = "'lexer': two '_' characters immediately one after another are not allowed";
	template<> const char* error::error_template<2002> = "'lexer': invalid number format - cannot declare a number with more that one '.' character";
	template<> const char* error::error_template<2003> = "'lexer': invalid number format - cannot declare an unsigned number containing '.' characters";
	template<> const char* error::error_template<2004> = "'lexer': invalid number format - floating point number must contain a '.' character";
	template<> const char* error::error_template<2005> = "'lexer': invalid unterminated character literal detected";
	template<> const char* error::error_template<2006> = "'lexer': invalid unterminated string literal detected";

	// *********************************************************************************************************************
	// parser 
	// *********************************************************************************************************************
	template<> const char* error::error_template<3000> = "'parser': unexpected token received (expected '{}', but received '{}' instead)";
	template<> const char* error::error_template<3001> = "'parser': unhandled token received ('{}')";
	template<> const char* error::error_template<3002> = "'parser': unhandled non-numerical token received ('{}')";
	template<> const char* error::error_template<3003> = "'parser': unexpected non-type token received ('{}')";
	template<> const char* error::error_template<3004> = "'parser': cannot apply a unary operator to a non-identifier value";

	// *********************************************************************************************************************
	// codegen errors
	// *********************************************************************************************************************
	template<> const char* error::error_template<4000> = "'{}': function has already been defined before";
	template<> const char* error::error_template<4001> = "'{}': function cannot be found";
	template<> const char* error::error_template<4002> = "'{}': function argument count mismatch";
	template<> const char* error::error_template<4003> = "'{}': variable cannot be found";
	template<> const char* error::error_template<4004> = "'{}': local variable has already been defined in the global scope";
	template<> const char* error::error_template<4005> = "'{}': local variable has already been defined before";
	template<> const char* error::error_template<4006> = "'{}': global variable has already been defined before";
	template<> const char* error::error_template<4007> = "'{}': return statement type '{}' does not match the expected function return type '{}'";
	template<> const char* error::error_template<4010> = "for conditional operator has to be of type 'bool' (received '{}' instead)";
	template<> const char* error::error_template<4011> = "invalid break statement detected outside a valid loop body";
	template<> const char* error::error_template<4012> = "unable to locate the main entry point";
	template<> const char* error::error_template<4013> = "unable to declare a main entry point with the return type of '{}' (expecting 'i32')";
	template<> const char* error::error_template<4014> = "unable to declare a numerical literal using a pointer type";
	template<> const char* error::error_template<4015> = "unable to declare a numerical literal using the given type '{}'";
	template<> const char* error::error_template<4016> = "IR module contains errors";
	template<> const char* error::error_template<4017> = "'{}': global variable has already been defined before at {}";
	// unary errors (4100)
	template<> const char* error::error_template<4100> = "'post decrement': operator expected a numerical value, but got '{}' instead";
	template<> const char* error::error_template<4101> = "'post increment': operator expected a numerical value, but got '{}' instead";
	template<> const char* error::error_template<4102> = "'pre decrement': operator expected a numerical value, but got '{}' instead";
	template<> const char* error::error_template<4103> = "'pre increment': operator expected a numerical value, but got '{}' instead";
	template<> const char* error::error_template<4104> = "'negation': operator expected a scalar value, but got '{}' instead";
	template<> const char* error::error_template<4105> = "'bitwise not': operation expects an integral, but received '{}' instead";
	// binary errors (4200)
	template<> const char* error::error_template<4200> = "'conjunction': operation expects 2 booleans, but received '{}' and '{}' instead";
	template<> const char* error::error_template<4201> = "'disjunction': operation expects 2 booleans, but received '{}' and '{}' instead";
	template<> const char* error::error_template<4202> = "'bitwise and': operation expects 2 integrals, but received '{}' and '{}' instead";
	template<> const char* error::error_template<4203> = "'bitwise or': operation expects 2 integrals, but received '{}' and '{}' instead";
	template<> const char* error::error_template<4204> = "'bitwise left shift': operation expects 2 integrals, but received '{}' and '{}' instead";
	template<> const char* error::error_template<4205> = "'bitwise right shift': operation expects 2 integrals, but received '{}' and '{}' instead";
	template<> const char* error::error_template<4206> = "'bitwise xor': operation expects 2 integrals, but received '{}' and '{}' instead";

	// *********************************************************************************************************************
	// compiler errors
	// *********************************************************************************************************************
	template<> const char* error::error_template<5000> = "the target machine cannot emit a file of this type";
	template<> const char* error::error_template<5001> = "clang compilation contains errors";
	// dependency graph errors (5100)
	template<> const char* error::error_template<5100> = "circular dependency detected";
	template<> const char* error::error_template<5101> = "invalid include statement detected - include directives are only allowed at the top of the source file";

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
				std::shared_ptr<error_message> error
			) : m_error(error) {}

			/**
			 * \brief Returns the contained error.
			 */
			std::shared_ptr<error_message> get_error() const {
				return m_error;
			}
		private:
			std::shared_ptr<error_message> m_error;
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
			const std::shared_ptr<error_message> get_error() const {
				return m_value.error();
			}
		private:
			std::expected<type, std::shared_ptr<error_message>> m_value;
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

			std::shared_ptr<error_message> get_error() const {
				return m_value.error();
			}
		private:
			std::expected<void, std::shared_ptr<error_message>> m_value;
		};
	}
}
