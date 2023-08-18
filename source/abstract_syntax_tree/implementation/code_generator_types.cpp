#include "code_generator.h"

#include "abstract_syntax_tree/keywords/types/char_node.h"
#include "abstract_syntax_tree/keywords/types/string_node.h"
#include "abstract_syntax_tree/keywords/types/bool_node.h"
#include "abstract_syntax_tree/keywords/types/numerical_literal_node.h"

namespace sigma {
	outcome::result<value_ptr> code_generator::visit_numerical_literal_node(
		numerical_literal_node& node,
		const code_generation_context& context
	) {
		const type contextually_derived_type = context.get_expected_type();
		const type literal_type = contextually_derived_type == type::unknown() ? node.get_preferred_type() : contextually_derived_type;

		if (literal_type.get_pointer_level() > 0) {
			return outcome::failure(
				error::emit<error_code::cannot_declare_numerical_using_pointer>(
					file_range{} // node.get_declared_position()
				)
			); // return on failure
		}

		switch (literal_type.get_base()) {
			// signed integers
		case type::base::i8:
			return std::make_shared<value>(
				"__i8", 
				type(type::base::i8, 0), 
				llvm::ConstantInt::get(
					m_context->get_context(),
					llvm::APInt(8, std::stoll(node.get_value()), true)
				)
			);
		case type::base::i16:
			return std::make_shared<value>(
				"__i16", 
				type(type::base::i16, 0),
				llvm::ConstantInt::get(
					m_context->get_context(), 
					llvm::APInt(16, std::stoll(node.get_value()), true)
				)
			);
		case type::base::i32:
			return std::make_shared<value>(
				"__i32", 
				type(type::base::i32, 0), 
				llvm::ConstantInt::get(
					m_context->get_context(), 
					llvm::APInt(32, std::stoll(node.get_value()), true)
				)
			);
		case type::base::i64:
			return std::make_shared<value>(
				"__i64", 
				type(type::base::i64, 0), 
				llvm::ConstantInt::get(
					m_context->get_context(),
					llvm::APInt(64, std::stoll(node.get_value()), true)
				)
			);
			// unsigned integers
		case type::base::u8:
			return std::make_shared<value>(
				"__u8",
				type(type::base::u8, 0),
				llvm::ConstantInt::get(
					m_context->get_context(), 
					llvm::APInt(8, std::stoull(node.get_value()), false)
				)
			);
		case type::base::u16:
			return std::make_shared<value>(
				"__u16", 
				type(type::base::u16, 0), 
				llvm::ConstantInt::get(
					m_context->get_context(),
					llvm::APInt(16, std::stoull(node.get_value()), false)
				)
			);
		case type::base::u32:
			return std::make_shared<value>(
				"__u32", 
				type(type::base::u32, 0),
				llvm::ConstantInt::get(
					m_context->get_context(),
					llvm::APInt(32, std::stoull(node.get_value()), false)
				)
			);
		case type::base::u64:
			return std::make_shared<value>(
				"__u64",
				type(type::base::u64, 0),
				llvm::ConstantInt::get(
					m_context->get_context(),
					llvm::APInt(64, std::stoull(node.get_value()), false)
				)
			);
			// floating point
		case type::base::f32:
			return std::make_shared<value>(
				"__f32",
				type(type::base::f32, 0),
				llvm::ConstantFP::get(
					m_context->get_context(),
					llvm::APFloat(std::stof(node.get_value()))
				)
			);
		case type::base::f64:
			return std::make_shared<value>(
				"__f64",
				type(type::base::f64, 0),
				llvm::ConstantFP::get(
					m_context->get_context(), 
					llvm::APFloat(std::stod(node.get_value()))
				)
			);
			// boolean
		case type::base::boolean:
			return create_boolean(
				std::stoi(node.get_value())
			);
		case type::base::character:
			return create_character(
				static_cast<char>(
					std::stoi(node.get_value())
				)
			);
		default:
			return outcome::failure(
				error::emit<error_code::cannot_declare_numerical_using_type>(
					file_range{}, //node.get_declared_position(), 
					literal_type
				)
			); // return on failure
		}
	}

	outcome::result<value_ptr> code_generator::visit_keyword_string_node(
		string_node& node,
		const code_generation_context& context
	) {
		SUPPRESS_C4100(context);

		// note: + 1 is for the null termination character
		const u64 string_length = node.get_value().size() + 1;

		// allocate memory for the string literal as a global constant
		llvm::ArrayType* array_type = llvm::ArrayType::get(
			type(type::base::character, 0).get_llvm_type(m_context->get_context()), 
			string_length
		);

		llvm::Constant* string_constant = llvm::ConstantDataArray::getString(
			m_context->get_context(), 
			node.get_value()
		);

		// create a global variable to store the string constant
		auto* global_string_literal = new llvm::GlobalVariable(
			*m_context->get_module(),
			array_type,
			true,
			llvm::GlobalValue::ExternalLinkage,
			string_constant,
			""
		);

		// bit cast the pointer to the global variable into a char* (i8*)
		llvm::Value* string_literal_ptr = m_context->get_builder().CreateBitCast(
			global_string_literal,
			type(type::base::character, 1).get_llvm_type(m_context->get_context())
		);

		return std::make_shared<value>(
			"__string", 
			type(type::base::character, 1), 
			string_literal_ptr
		);
	}

	outcome::result<value_ptr> code_generator::visit_keyword_char_node(
		char_node& node,
		const code_generation_context& context
	) {
		SUPPRESS_C4100(context);

		return create_character(node.get_value());
	}

	outcome::result<value_ptr> code_generator::visit_keyword_bool_node(
		bool_node& node, 
		const code_generation_context& context
	) {
		SUPPRESS_C4100(context);

		return create_boolean(node.get_value());
	}

	value_ptr code_generator::create_boolean(bool val) const {
		return std::make_shared<value>(
			"__bool",
			type(type::base::boolean, 0),
			llvm::ConstantInt::get(
				m_context->get_context(),
				llvm::APInt(1, val, false)
			)
		);
	}

	value_ptr code_generator::create_character(char val) const {
		return std::make_shared<value>(
			"__char",
			type(type::base::character, 0),
			llvm::ConstantInt::get(
				m_context->get_context(),
				llvm::APInt(8, static_cast<u64>(val), false)
			)
		);
	}
}
