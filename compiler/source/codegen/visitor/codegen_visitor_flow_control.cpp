#include "codegen_visitor.h"

#include "codegen/abstract_syntax_tree/keywords/flow_control/return_node.h"
#include "codegen/abstract_syntax_tree/keywords/flow_control/if_else_node.h"
#include "codegen/abstract_syntax_tree/keywords/flow_control/while_node.h"
#include "codegen/abstract_syntax_tree/keywords/flow_control/for_node.h"
#include "codegen/abstract_syntax_tree/keywords/flow_control/break_node.h"

namespace channel {
	bool codegen_visitor::visit_return_node(return_node& node, value_ptr& out_value, codegen_context context) {
		// evaluate the expression of the return statement
		value_ptr return_value;
		if (!node.get_return_expression_node()->accept(*this, return_value, {})) {
			return false;
		}

		// get the return type of the current function
		const llvm::Function* parent_function = m_builder.GetInsertBlock()->getParent();
		const type function_return_type = m_function_registry.get_function(parent_function->getName().str())->get_return_type();

		// upcast the return value to match the function's return type
		llvm::Value* upcasted_return_value;
		if(!cast_value(upcasted_return_value, return_value, function_return_type, node.get_declared_position())) {
			return false;
		}

		// generate the LLVM return instruction with the upcasted value
		m_builder.CreateRet(upcasted_return_value);

		// return the value of the expression (use upcasted value's type)
        out_value = std::make_shared<value>("__return", function_return_type, upcasted_return_value);
		return true;
	}

	bool codegen_visitor::visit_if_else_node(if_else_node& node, value_ptr& out_value, codegen_context context) {
        llvm::BasicBlock* entry_block = m_builder.GetInsertBlock();
        llvm::Function* parent_function = entry_block->getParent();

        llvm::BasicBlock* end_block = llvm::BasicBlock::Create(m_context, "", parent_function);

        // get condition and branch nodes
        const auto& condition_nodes = node.get_condition_nodes();
        const auto& branch_nodes = node.get_branch_nodes();

        // determine if there is a trailing else block
        const bool has_trailing_else = condition_nodes.back() == nullptr;
        const u64 condition_node_count = has_trailing_else ? condition_nodes.size() - 2 : condition_nodes.size() - 1;

        // initialize condition and branch blocks with their respective sizes
        std::vector<llvm::BasicBlock*> condition_blocks(condition_node_count);
        std::vector<llvm::BasicBlock*> branch_blocks(branch_nodes.size());

        // create condition blocks
        for (u64 i = 0; i < condition_node_count; ++i) {
            condition_blocks[i] = llvm::BasicBlock::Create(m_context, "", parent_function);
        }

        // create branch blocks
        for (u64 i = 0; i < branch_nodes.size(); ++i) {
            branch_blocks[i] = llvm::BasicBlock::Create(m_context, "", parent_function);
        }

        // accept the first condition
        value_ptr condition_value;
        if (!condition_nodes[0]->accept(*this, condition_value, {})) {
            return false;
        }

        // create a conditional branch based on the first condition
        m_builder.CreateCondBr(
            condition_value->get_value(),
            branch_blocks[0],
            condition_blocks.empty() ? (has_trailing_else ? branch_blocks.back() : end_block) : condition_blocks[0]
        );

        // process remaining conditions and create appropriate branches
        for (u64 i = 0; i < condition_node_count; ++i) {
            m_builder.SetInsertPoint(condition_blocks[i]);

            if (!condition_nodes[i + 1]->accept(*this, condition_value, {})) {
                return false;
            }

            m_builder.CreateCondBr(
                condition_value->get_value(),
                branch_blocks[i + 1],
                i < condition_node_count - 1 ? condition_blocks[i + 1] : branch_blocks.back()
            );
        }

        // save the previous scope
        scope* prev_scope = m_scope;

        // process branch nodes and create appropriate inner statements
        for (u64 i = 0; i < branch_nodes.size(); ++i) {
            m_builder.SetInsertPoint(branch_blocks[i]);
            m_scope = new scope(prev_scope, nullptr);

            for (const auto& statement : branch_nodes[i]) {
                value_ptr temp_statement_value;
                if (!statement->accept(*this, temp_statement_value, {})) {
                    return false;
                }
            }

            if(!m_builder.GetInsertBlock()->getTerminator()) {
                m_builder.CreateBr(end_block);
            }
        }

        // restore the previous scope and set the insert point to the end block
        m_scope = prev_scope;
        m_builder.SetInsertPoint(end_block);
        out_value = nullptr;
        return true;
	}

    bool codegen_visitor::visit_while_node(while_node& node, value_ptr& out_value, codegen_context context) {
        llvm::BasicBlock* entry_block = m_builder.GetInsertBlock();
        llvm::Function* parent_function = entry_block->getParent();

        llvm::BasicBlock* end_block = llvm::BasicBlock::Create(m_context, "", parent_function);
        llvm::BasicBlock* condition_block = llvm::BasicBlock::Create(m_context, "", parent_function);
        llvm::BasicBlock* loop_body_block = llvm::BasicBlock::Create(m_context, "", parent_function);

        m_builder.CreateBr(condition_block);

        // condition block
        m_builder.SetInsertPoint(condition_block);

        // accept the condition node
        value_ptr condition_value;
        if (!node.get_loop_condition_node()->accept(*this, condition_value, {})) {
            return false;
        }

        m_builder.CreateCondBr(
            condition_value->get_value(),
            loop_body_block,
            end_block
        );

        // accept all statements in the loop body
        m_builder.SetInsertPoint(loop_body_block);

        // save the previous scope
        scope* prev_scope = m_scope;
        m_scope = new scope(prev_scope, end_block);

        for(channel::node* n : node.get_loop_body_nodes()) {
            value_ptr temp_value;
            if(!n->accept(*this, temp_value, {})) {
                return false;
            }
        }

        // restore the previous scope and set the insert point to the end block
        m_scope = prev_scope;

        // only add a terminator block if we don't have one
        if (!m_builder.GetInsertBlock()->getTerminator()) {
            m_builder.CreateBr(condition_block);
        }

        m_builder.SetInsertPoint(end_block);
        return nullptr;
        return true;
    }

    bool codegen_visitor::visit_for_node(for_node& node, value_ptr& out_value, codegen_context context) {
        llvm::BasicBlock* entry_block = m_builder.GetInsertBlock();
        llvm::Function* parent_function = entry_block->getParent();

        llvm::BasicBlock* end_block = llvm::BasicBlock::Create(m_context, "", parent_function);
        llvm::BasicBlock* condition_block = llvm::BasicBlock::Create(m_context, "", parent_function);
        llvm::BasicBlock* increment_block = llvm::BasicBlock::Create(m_context, "", parent_function);
        llvm::BasicBlock* loop_body_block = llvm::BasicBlock::Create(m_context, "", parent_function);

        value_ptr temp_value;

        // create the index expression
        if (!node.get_loop_initialization_node()->accept(*this, temp_value, {})) {
            return false;
        }

        m_builder.CreateBr(condition_block);

        // accept the condition block
        m_builder.SetInsertPoint(condition_block);
        value_ptr condition_value;
        if(!node.get_loop_condition_node()->accept(*this, condition_value, {})) {
            return false;
        }

        // check if the conditional operator evaluates to a boolean
        if(condition_value->get_type().get_base() != type::base::boolean || condition_value->get_type().is_pointer()) {
            error::emit<4010>(node.get_declared_position(), condition_value->get_type()).print();
            return false;
        }

        m_builder.CreateCondBr(
            condition_value->get_value(),
            loop_body_block,
            end_block
        );

        // create the increment block
        m_builder.SetInsertPoint(increment_block);
        for (channel::node* n : node.get_post_iteration_nodes()) {
            if (!n->accept(*this, temp_value, {})) {
                return false;
            }
        }

        m_builder.CreateBr(condition_block);

        // create the loop body block
        m_builder.SetInsertPoint(loop_body_block);

        // save the previous scope
        scope* prev_scope = m_scope;
        m_scope = new scope(prev_scope, end_block);

        // accept all inner statements
        for (channel::node* n : node.get_loop_body_nodes()) {
            if (!n->accept(*this, temp_value, {})) {
                return false;
            }
        }

        // restore the previous scope and set the insert point to the end block
        m_scope = prev_scope;

        // only add a terminator block if we don't have one
        if (!m_builder.GetInsertBlock()->getTerminator()) {
            m_builder.CreateBr(increment_block);
		}

        m_builder.SetInsertPoint(end_block);
        out_value = nullptr;
        return true;
    }

    bool codegen_visitor::visit_break_node(break_node& node, value_ptr& out_value, codegen_context context) {
        llvm::BasicBlock* end_block = m_scope->get_loop_end_block();
        if (end_block == nullptr) {
            // emit an error if there's no enclosing loop to break from
            error::emit<4011>(node.get_declared_position()).print();
            return false;
        }

        // only add a terminator block if we don't have one
        if (!m_builder.GetInsertBlock()->getTerminator()) {
            m_builder.CreateBr(end_block);
        }

        // create a new basic block for the remaining loop body and set it as the current insert point
        llvm::Function* parent_function = end_block->getParent();
        llvm::BasicBlock* continue_block = llvm::BasicBlock::Create(m_context, "", parent_function);
        m_builder.SetInsertPoint(continue_block);

        out_value = nullptr;
        return true;
    }
}