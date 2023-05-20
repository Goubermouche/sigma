#pragma once
#include "compiler/diagnostics/diagnostic.h"

namespace channel {
	constexpr auto error_templates = make_hash_map(
		// *********************************************************************************************************************
		// system
		// *********************************************************************************************************************
		std::pair{ 1000, "'{}': unable to open file" },
		std::pair{ 1001, "'{}': unable to delete file" },
		std::pair{ 1002, "'{}': file does not exist" },
		std::pair{ 1003, "'{}': directory was not expected (expected a file)" },
		std::pair{ 1004, "'{}': file was not expected (expected a directory)" },
		std::pair{ 1005, "'{}': unable to seek end of file" },
		std::pair{ 1006, "'{}': unable to read file" },
		std::pair{ 1007, "'{}': invalid file extension (expected '{}')" },
		std::pair{ 1008, "" },
		std::pair{ 1009, "" },
		// *********************************************************************************************************************
		// lexer
		// *********************************************************************************************************************
		std::pair{ 2000, "'lexer': invalid '.' character detected at token start" },
		std::pair{ 2001, "'lexer': two '_' characters immediately one after another are not allowed" },
		std::pair{ 2002, "'lexer': invalid number format - cannot declare a number with more that one '.' character" },
		std::pair{ 2003, "'lexer': invalid number format - cannot declare an unsigned number containing '.' characters" },
		std::pair{ 2004, "'lexer': invalid number format - floating point number must contain a '.' character" },
		std::pair{ 2005, "'lexer': invalid unterminated character literal detected" },
		std::pair{ 2006, "'lexer': invalid unterminated string literal detected" },
		// *********************************************************************************************************************
		// parser 
		// *********************************************************************************************************************
		std::pair{ 3000, "'parser': unexpected token received (expected '{}', but received '{}' instead)" },
		std::pair{ 3001, "'parser': unhandled token received ('{}')" },
		std::pair{ 3002, "'parser': unhandled non-numerical token received ('{}')" },
		std::pair{ 3003, "'parser': unexpected non-type token received ('{}')" },
		std::pair{ 3004, "'parser': cannot apply a unary operator to a non-identifier value" },
		// *********************************************************************************************************************
		// codegen errors
		// *********************************************************************************************************************
		std::pair{ 4000, "'{}': function has already been defined before" },
		std::pair{ 4001, "'{}': function cannot be found" },
		std::pair{ 4002, "'{}': function argument count mismatch" },
		std::pair{ 4003, "'{}': variable cannot be found" },
		std::pair{ 4004, "'{}': local variable has already been defined in the global scope" },
		std::pair{ 4005, "'{}': local variable has already been defined before" },
		std::pair{ 4006, "'{}': global variable has already been defined before" },
		// unary errors (4100)
		std::pair{ 4100, "'post decrement': operator expected a numerical value, but got '{}' instead" },
		std::pair{ 4101, "'post increment': operator expected a numerical value, but got '{}' instead" },
		std::pair{ 4102, "'pre decrement': operator expected a numerical value, but got '{}' instead" },
		std::pair{ 4103, "'pre increment': operator expected a numerical value, but got '{}' instead" },
		std::pair{ 4104, "'negation': operator expected a scalar value, but got '{}' instead" },
		std::pair{ 4105, "'bitwise not': operation expects an integral, but received '{}' instead" },
		// binary errors (4200)
		std::pair{ 4200, "'conjunction': operation expects 2 booleans, but received '{}' and '{}' instead" },
		std::pair{ 4201, "'disjunction': operation expects 2 booleans, but received '{}' and '{}' instead" },
		std::pair{ 4202, "'bitwise and': operation expects 2 integrals, but received '{}' and '{}' instead" },
		std::pair{ 4203, "'bitwise or': operation expects 2 integrals, but received '{}' and '{}' instead" },
		std::pair{ 4204, "'bitwise left shift': operation expects 2 integrals, but received '{}' and '{}' instead" },
		std::pair{ 4205, "'bitwise right shift': operation expects 2 integrals, but received '{}' and '{}' instead" },
		std::pair{ 4206, "'bitwise xor': operation expects 2 integrals, but received '{}' and '{}' instead" },

		std::pair{ 4010, "for conditional operator has to be of type 'bool' (received '{}' instead)" },
		std::pair{ 4011, "invalid break statement detected outside a valid loop body" },
		std::pair{ 4012, "unable to locate the main entry point" },
		std::pair{ 4013, "unable to declare a main entry point with the return type of '{}' (expecting 'i32')" },
		std::pair{ 4014, "unable to declare a numerical literal using a pointer type" },
		std::pair{ 4015, "unable to declare a numerical literal using the given type '{}'" },
		std::pair{ 4016, "IR module contains errors" },
		// *********************************************************************************************************************
		// compiler errors
		// *********************************************************************************************************************
		std::pair{ 5000, "the target machine cannot emit a file of this type" },
		std::pair{ 5001, "clang compilation contains errors" },
		std::pair{ 5002, "" }
	);

	class error_message : public diagnostic_message {
	public:
		error_message(std::string message, u64 code);

		void print() const override;
	};

	class error_message_position : public error_message {
	public:
		error_message_position(std::string message, u64 code, token_position position);

		void print() const override;
	protected:
		token_position m_position;
	};

	class error {
	public:
		template <u64 code, typename... Args>
		static error_message emit(Args&&... args);

		template <u64 code, typename... Args>
		static error_message_position emit(token_position position, Args&&... args);
	};

	template<u64 code, typename ...Args>
	error_message error::emit(Args&& ...args) {
		return {
			std::move(std::format(error_templates[code], std::forward<Args>(args)...)),
			code
		};
	}

	template<u64 code, typename ...Args>
	error_message_position error::emit(token_position position, Args && ...args) {
		return {
			std::move(std::format(error_templates[code], std::forward<Args>(args)...)),
			code,
			position
		};
	}

	/**
	 * \brief Potential (optional) erroneous result.
	 */
	using error_result = std::optional<error_message>;
}
