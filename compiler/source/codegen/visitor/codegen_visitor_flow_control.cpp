#include "codegen_visitor.h"

#include "../abstract_syntax_tree/keywords/flow_control/return_node.h"

namespace channel {
	bool codegen_visitor::visit_return_node(return_node& node, value*& out_value) {
		// evaluate the expression of the return statement
		value* return_value;
		if (!node.get_expression()->accept(*this, return_value)) {
			return false;
		}

		// get the return type of the current function
		const llvm::Function* parent_function = m_builder.GetInsertBlock()->getParent();
		const type function_return_type = m_functions[parent_function->getName().str()]->get_return_type();

		// upcast the return value to match the function's return type
		llvm::Value* upcasted_return_value = cast_value(return_value, function_return_type, node.get_declaration_line_number());

		// generate the LLVM return instruction with the upcasted value
		m_builder.CreateRet(upcasted_return_value);

		// return the value of the expression (use upcasted value's type)
		out_value = new value("__return", function_return_type, upcasted_return_value);
		return true;
	}
}