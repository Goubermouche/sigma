#include "codegen_visitor.h"

// signed integer
#include "../abstract_syntax_tree/keywords/types/signed_int/i8_node.h"
#include "../abstract_syntax_tree/keywords/types/signed_int/i16_node.h"
#include "../abstract_syntax_tree/keywords/types/signed_int/i32_node.h"
#include "../abstract_syntax_tree/keywords/types/signed_int/i64_node.h"
// unsigned integer
#include "../abstract_syntax_tree/keywords/types/unsigned_int/u8_node.h"
#include "../abstract_syntax_tree/keywords/types/unsigned_int/u16_node.h"
#include "../abstract_syntax_tree/keywords/types/unsigned_int/u32_node.h"
#include "../abstract_syntax_tree/keywords/types/unsigned_int/u64_node.h"
// floating point
#include "../abstract_syntax_tree/keywords/types/floating_point/f32_node.h"
#include "../abstract_syntax_tree/keywords/types/floating_point/f64_node.h"
// text
#include "../abstract_syntax_tree/keywords/types/text/char_node.h"
#include "../abstract_syntax_tree/keywords/types/text/string_node.h"
// other
#include "../abstract_syntax_tree/keywords/types/bool_node.h"

namespace channel {
	bool codegen_visitor::visit_keyword_i8_node(i8_node& node, value_ptr& out_value) {
		out_value = std::make_shared<value>("__i8", type(type::base::i8, 0), llvm::ConstantInt::get(m_context, llvm::APInt(8, node.get_value(), true)));
		return true;
	}

	bool codegen_visitor::visit_keyword_i16_node(i16_node& node, value_ptr& out_value) {
		out_value = std::make_shared<value>("__i16", type(type::base::i16, 0), llvm::ConstantInt::get(m_context, llvm::APInt(16, node.get_value(), true)));
		return true;
	}

	bool codegen_visitor::visit_keyword_i32_node(i32_node& node, value_ptr& out_value) {
		out_value = std::make_shared<value>("__i32", type(type::base::i32, 0), llvm::ConstantInt::get(m_context, llvm::APInt(32, node.get_value(), true)));
		return true;
	}

	bool codegen_visitor::visit_keyword_i64_node(i64_node& node, value_ptr& out_value) {
		out_value = std::make_shared<value>("__i64", type(type::base::i64, 0), llvm::ConstantInt::get(m_context, llvm::APInt(64, node.get_value(), true)));
		return true;
	}

	bool codegen_visitor::visit_keyword_u8_node(u8_node& node, value_ptr& out_value) {
		out_value = std::make_shared<value>("__u8", type(type::base::u8, 0), llvm::ConstantInt::get(m_context, llvm::APInt(8, node.get_value(), false)));
		return true;
	}

	bool codegen_visitor::visit_keyword_u16_node(u16_node& node, value_ptr& out_value) {
		out_value = std::make_shared<value>("__u16", type(type::base::u16, 0), llvm::ConstantInt::get(m_context, llvm::APInt(16, node.get_value(), false)));
		return true;
	}

	bool codegen_visitor::visit_keyword_u32_node(u32_node& node, value_ptr& out_value) {
		out_value = std::make_shared<value>("__u32", type(type::base::u32, 0), llvm::ConstantInt::get(m_context, llvm::APInt(32, node.get_value(), false)));
		return true;
	}

	bool codegen_visitor::visit_keyword_u64_node(u64_node& node, value_ptr& out_value) {
		out_value = std::make_shared<value>("__u64", type(type::base::u64, 0), llvm::ConstantInt::get(m_context, llvm::APInt(64, node.get_value(), false)));
		return true;
	}

	bool codegen_visitor::visit_keyword_f32_node(f32_node& node, value_ptr& out_value) {
		out_value = std::make_shared<value>("__f32", type(type::base::f32, 0), llvm::ConstantFP::get(m_context, llvm::APFloat(node.get_value())));
		return true;
	}

	bool codegen_visitor::visit_keyword_f64_node(f64_node& node, value_ptr& out_value) {
		out_value = std::make_shared<value>("__f64", type(type::base::f64, 0), llvm::ConstantFP::get(m_context, llvm::APFloat(node.get_value())));
		return true;
	}

	bool codegen_visitor::visit_keyword_char_node(char_node& node, value_ptr& out_value) {
		out_value = std::make_shared<value>("__char", type(type::base::character, 0), llvm::ConstantInt::get(m_context, llvm::APInt(8, static_cast<u64>(node.get_value()), false)));
		return true;
	}

	bool codegen_visitor::visit_keyword_string_node(string_node& node, value_ptr& out_value) {
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

	bool codegen_visitor::visit_keyword_bool_node(bool_node& node, value_ptr& out_value) {
		out_value = std::make_shared<value>("__bool", type(type::base::boolean, 0), llvm::ConstantInt::get(m_context, llvm::APInt(1, node.get_value(), false)));
		return true;
	}
}