#include "codegen_visitor.h"

#include "../abstract_syntax_tree/keywords/types/signed_int/keyword_i8_node.h"
#include "../abstract_syntax_tree/keywords/types/signed_int/keyword_i16_node.h"
#include "../abstract_syntax_tree/keywords/types/signed_int/keyword_i32_node.h"
#include "../abstract_syntax_tree/keywords/types/signed_int/keyword_i64_node.h"

#include "../abstract_syntax_tree/keywords/types/unsigned_int/keyword_u8_node.h"
#include "../abstract_syntax_tree/keywords/types/unsigned_int/keyword_u16_node.h"
#include "../abstract_syntax_tree/keywords/types/unsigned_int/keyword_u32_node.h"
#include "../abstract_syntax_tree/keywords/types/unsigned_int/keyword_u64_node.h"

#include "../abstract_syntax_tree/keywords/types/floating_point/keyword_f32_node.h"
#include "../abstract_syntax_tree/keywords/types/floating_point/keyword_f64_node.h"

namespace channel {
	bool codegen_visitor::visit_keyword_i8_node(keyword_i8_node& node, value*& out_value) {
		out_value = new value("__i8", type::i8, llvm::ConstantInt::get(m_context, llvm::APInt(8, node.get_value(), true)));
		return true;
	}

	bool codegen_visitor::visit_keyword_i16_node(keyword_i16_node& node, value*& out_value) {
		out_value = new value("__i16", type::i16, llvm::ConstantInt::get(m_context, llvm::APInt(16, node.get_value(), true)));
		return true;
	}

	bool codegen_visitor::visit_keyword_i32_node(keyword_i32_node& node, value*& out_value) {
		out_value = new value("__i32", type::i32, llvm::ConstantInt::get(m_context, llvm::APInt(32, node.get_value(), true)));
		return true;
	}

	bool codegen_visitor::visit_keyword_i64_node(keyword_i64_node& node, value*& out_value) {
		out_value = new value("__i64", type::i64, llvm::ConstantInt::get(m_context, llvm::APInt(64, node.get_value(), true)));
		return true;
	}

	bool codegen_visitor::visit_keyword_u8_node(keyword_u8_node& node, value*& out_value) {
		out_value = new value("__u8", type::u8, llvm::ConstantInt::get(m_context, llvm::APInt(8, node.get_value(), false)));
		return true;
	}

	bool codegen_visitor::visit_keyword_u16_node(keyword_u16_node& node, value*& out_value) {
		out_value = new value("__u16", type::u16, llvm::ConstantInt::get(m_context, llvm::APInt(16, node.get_value(), false)));
		return true;
	}

	bool codegen_visitor::visit_keyword_u32_node(keyword_u32_node& node, value*& out_value) {
		out_value = new value("__u32", type::u32, llvm::ConstantInt::get(m_context, llvm::APInt(32, node.get_value(), false)));
		return true;
	}

	bool codegen_visitor::visit_keyword_u64_node(keyword_u64_node& node, value*& out_value) {
		out_value = new value("__u64", type::u64, llvm::ConstantInt::get(m_context, llvm::APInt(64, node.get_value(), false)));
		return true;
	}

	bool codegen_visitor::visit_keyword_f32_node(keyword_f32_node& node, value*& out_value) {
		out_value = new value("__f32", type::f32, llvm::ConstantFP::get(m_context, llvm::APFloat(node.get_value())));
		return true;
	}

	bool codegen_visitor::visit_keyword_f64_node(keyword_f64_node& node, value*& out_value) {
		out_value = new value("__f64", type::f64, llvm::ConstantFP::get(m_context, llvm::APFloat(node.get_value())));
		return true;
	}
}