#include "scope.h"

namespace sigma {
	scope::scope(
		scope_ptr parent
	) : m_parent(parent) {}

	scope::scope(
		scope_ptr parent,
		llvm::BasicBlock* loop_end_block
	) : m_parent(parent),
	m_loop_end_block(loop_end_block) {}

	value_ptr scope::get_variable(
		const std::string& variable_name
	) {
		// try to find an llvm::Value in this scope
		if(value_ptr value = m_variables.get(variable_name)) {
			return value;
		}

		// if we haven't found a value, but we have a parent scope we need to search it as well 
		if (m_parent != nullptr) {
			return m_parent->get_variable(variable_name);
		}

		// value does not exist in current scope
		return nullptr;
	}

	bool scope::contains_variable(
		const std::string& variable_name
	) const	{
		// check in the current scopes
		if (m_variables.contains(variable_name)) {
			return true;
		}

		// if we have a parent scope check in there as well
		if (m_parent != nullptr) {
			return m_parent->contains_variable(variable_name);
		}

		// variable does not exist in the current scope
		return false;
	}

	bool scope::insert_variable(
		const std::string& variable_name, 
		value_ptr value
	) {
		// check parent scopes
		if (contains_variable(variable_name)) {
			return false;
		}

		return m_variables.insert(variable_name, value);
	}

	llvm::BasicBlock* scope::get_loop_end_block() const {
		if (m_loop_end_block != nullptr) {
			return m_loop_end_block;
		}

		if (m_parent != nullptr) {
			return m_parent->get_loop_end_block();
		}

		return nullptr;
	}
}