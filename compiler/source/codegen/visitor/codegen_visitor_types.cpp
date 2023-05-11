#include "codegen_visitor.h"

#include "codegen/abstract_syntax_tree/keywords/types/char_node.h"
#include "codegen/abstract_syntax_tree/keywords/types/string_node.h"
#include "codegen/abstract_syntax_tree/keywords/types/bool_node.h"
#include "codegen/abstract_syntax_tree/keywords/types/numerical_literal_node.h"

namespace channel {
	bool codegen_visitor::visit_numerical_literal_node(numerical_literal_node& node, value_ptr& out_value, codegen_context context) {
		const type contextually_derived_type = context.get_expected_type();
		const type literal_type = contextually_derived_type == type::unknown() ? node.get_preferred_type() : contextually_derived_type;

		if (literal_type.get_pointer_level() > 0) {
			error::emit<4014>(node.get_declared_position()).print();
			return false;
		}

		switch (literal_type.get_base()) {
			// signed integers
		case type::base::i8:
			out_value = std::make_shared<value>("__i8", type(type::base::i8, 0), llvm::ConstantInt::get(m_context, llvm::APInt(8, std::stoll(node.get_value()), true)));
			return true;
		case type::base::i16:
			out_value = std::make_shared<value>("__i16", type(type::base::i16, 0), llvm::ConstantInt::get(m_context, llvm::APInt(16, std::stoll(node.get_value()), true)));
			return true;
		case type::base::i32:
			out_value = std::make_shared<value>("__i32", type(type::base::i32, 0), llvm::ConstantInt::get(m_context, llvm::APInt(32, std::stoll(node.get_value()), true)));
			return true;
		case type::base::i64:
			out_value = std::make_shared<value>("__i64", type(type::base::i64, 0), llvm::ConstantInt::get(m_context, llvm::APInt(64, std::stoll(node.get_value()), true)));
			return true;
			// unsigned integers
		case type::base::u8:
			out_value = std::make_shared<value>("__u8", type(type::base::u8, 0), llvm::ConstantInt::get(m_context, llvm::APInt(8, std::stoull(node.get_value()), false)));
			return true;
		case type::base::u16:
			out_value = std::make_shared<value>("__u16", type(type::base::u16, 0), llvm::ConstantInt::get(m_context, llvm::APInt(16, std::stoull(node.get_value()), false)));
			return true;
		case type::base::u32:
			out_value = std::make_shared<value>("__u32", type(type::base::u32, 0), llvm::ConstantInt::get(m_context, llvm::APInt(32, std::stoull(node.get_value()), false)));
			return true;
		case type::base::u64:
			out_value = std::make_shared<value>("__u64", type(type::base::u64, 0), llvm::ConstantInt::get(m_context, llvm::APInt(64, std::stoull(node.get_value()), false)));
			return true;
			// floating point
		case type::base::f32:
			out_value = std::make_shared<value>("__f32", type(type::base::f32, 0), llvm::ConstantFP::get(m_context, llvm::APFloat(std::stof(node.get_value()))));
			return true;
		case type::base::f64:
			out_value = std::make_shared<value>("__f64", type(type::base::f64, 0), llvm::ConstantFP::get(m_context, llvm::APFloat(std::stod(node.get_value()))));
			return true;

		default:
			std::cout << "!!!! unexpected type : " << literal_type.to_string() << '\n';
			return false; // return on failure
		}
	}

	bool codegen_visitor::visit_keyword_char_node(char_node& node, value_ptr& out_value, codegen_context context) {
		out_value = std::make_shared<value>("__char", type(type::base::character, 0), llvm::ConstantInt::get(m_context, llvm::APInt(8, static_cast<u64>(node.get_value()), false)));
		return true;
	}

	bool codegen_visitor::visit_keyword_string_node(string_node& node, value_ptr& out_value, codegen_context context) {
		// note: + 1 is for the null termination character
		const u64 string_length = node.get_value().size() + 1;

		// allocate memory for the string literal as a global constant
		llvm::ArrayType* array_type = llvm::ArrayType::get(type(type::base::character, 0).get_llvm_type(m_context), string_length);
		llvm::Constant* string_constant = llvm::ConstantDataArray::getString(m_context, node.get_value());

		// create a global variable to store the string constant
		llvm::GlobalVariable* global_string_literal = new llvm::GlobalVariable(*m_module, array_type, true, llvm::GlobalValue::PrivateLinkage, string_constant, ".str");

		// bit cast the pointer to the global variable into a char* (i8*)
		llvm::Value* string_literal_ptr = m_builder.CreateBitCast(global_string_literal, type(type::base::character, 1).get_llvm_type(m_context));

		out_value = std::make_shared<value>("__string", type(type::base::character, 1), string_literal_ptr);
		return true;
	}

	bool codegen_visitor::visit_keyword_bool_node(bool_node& node, value_ptr& out_value, codegen_context context) {
		out_value = std::make_shared<value>("__bool", type(type::base::boolean, 0), llvm::ConstantInt::get(m_context, llvm::APInt(1, node.get_value(), false)));
		return true;
	}
}