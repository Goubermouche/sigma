#include "codegen_visitor.h"

#include "../abstract_syntax_tree/keywords/flow_control/return_node.h"
#include "../abstract_syntax_tree/keywords/flow_control/if_else_node.h"

namespace channel {
	bool codegen_visitor::visit_return_node(return_node& node, value*& out_value) {
		// evaluate the expression of the return statement
		value* return_value;
		if (!node.get_return_expression_node()->accept(*this, return_value)) {
			return false;
		}

		// get the return type of the current function
		const llvm::Function* parent_function = m_builder.GetInsertBlock()->getParent();
		const type function_return_type = m_functions[parent_function->getName().str()]->get_return_type();

		// upcast the return value to match the function's return type
		llvm::Value* upcasted_return_value;
		if(!cast_value(upcasted_return_value, return_value, function_return_type, node.get_declaration_line_number())) {
			return false;
		}

		// generate the LLVM return instruction with the upcasted value
		m_builder.CreateRet(upcasted_return_value);

		// return the value of the expression (use upcasted value's type)
		out_value = new value("__return", function_return_type, upcasted_return_value);
		return true;
	}

	bool codegen_visitor::visit_if_else_node(if_else_node& node, value*& out_value) {
        llvm::BasicBlock* entry_block = m_builder.GetInsertBlock();
        llvm::Function* parent_function = entry_block->getParent();
        llvm::BasicBlock* after_if_else_block = llvm::BasicBlock::Create(m_context, "", parent_function);

		const auto& condition_nodes = node.get_condition_nodes();
        const auto& branch_nodes = node.get_branch_nodes();

        const bool has_trailing_else = condition_nodes.back() == nullptr;
        const u64 condition_node_count = has_trailing_else ? condition_nodes.size() - 2 : condition_nodes.size() - 1;

        std::vector<llvm::BasicBlock*> condition_blocks;
        std::vector<llvm::BasicBlock*> branch_blocks;

        // create condition blocks
        for (u64 i = 0; i < condition_node_count; ++i) {
            condition_blocks.emplace_back(llvm::BasicBlock::Create(m_context, "", parent_function));
        }

        // create branch blocks
        for (u64 i = 0; i < branch_nodes.size(); ++i) {
            branch_blocks.emplace_back(llvm::BasicBlock::Create(m_context, "", parent_function));
        }

        // accept the first condition manually
        value* condition_value;
        if (!condition_nodes[0]->accept(*this, condition_value)) {
            return false;
        }

        // determine the next block 
        if(condition_blocks.empty()) {
            // we don't have other condition blocks, so the next block either has to be an else branch, or the end branch
            m_builder.CreateCondBr(
                condition_value->get_value(),
                branch_blocks[0],
                has_trailing_else ? branch_blocks.back() : after_if_else_block
            );
        }
        else {
            // we still have more condition blocks, use that as the fail-case
            m_builder.CreateCondBr(
                condition_value->get_value(),
                branch_blocks[0],
                condition_blocks[0]
            );
        }
       
        // accept other conditions
        for (u64 i = 0; i < condition_node_count; ++i) {
            m_builder.SetInsertPoint(condition_blocks[i]);

            // accept the condition
            if (!condition_nodes[i + 1]->accept(*this, condition_value)) {
                return false;
            }

            // determine the next block
            // check if we still have some available condition branches
            if(i < condition_node_count - 1) {
                m_builder.CreateCondBr(
                    condition_value->get_value(),
                    branch_blocks[i + 1], 
                    condition_blocks[i + 1] // fail case: go into the next condition branch
                );
            }
            else {
                m_builder.CreateCondBr(
                    condition_value->get_value(),
                    branch_blocks[i + 1],
                    branch_blocks.back() // fail case: go into the last branch block
                );
            }
        }

        scope* prev_scope = m_scope;

        // accepts inner statements for every condition
		for (u64 i = 0; i < branch_nodes.size(); ++i) {
            m_builder.SetInsertPoint(branch_blocks[i]);
            m_scope = new scope(prev_scope);

            for (const auto& statement : branch_nodes[i]) {
                value* temp_statement_value;
                if (!statement->accept(*this, temp_statement_value)) {
                    return false;
                }
            }

            // return to the end block
            m_builder.CreateBr(after_if_else_block);
        }

        m_scope = prev_scope;

        // use the end block as the new entry point
        m_builder.SetInsertPoint(after_if_else_block);
        out_value = nullptr;
        return true;
	}
}