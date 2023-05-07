#include "scope.h"

namespace channel {
	scope::scope(
		scope* parent,
		llvm::BasicBlock* loop_end_block
	) : m_parent(parent),
	m_loop_end_block(loop_end_block) {}

	void scope::insert_named_value(const std::string& name, value_ptr value) {
		m_named_values[name] = value;
	}

	value_ptr scope::get_named_value(const std::string& name) {
		const auto it = m_named_values.find(name); // try to find an llvm::Value in this scope

		// if we've found a value in this scope, return it
		if (it != m_named_values.end()) {
			return it->second;
		}

		// if we haven't found a value, but we have a parent scope we need to search it as well 
		if (m_parent != nullptr) {
			return m_parent->get_named_value(name);
		}

		// value does not exist in current scope
		return nullptr;
	}

	llvm::BasicBlock* scope::get_loop_end_block() const {
		if(m_loop_end_block != nullptr) {
			return m_loop_end_block;
		}

		if(m_parent != nullptr) {
			return m_parent->get_loop_end_block();
		}

		return nullptr;
	}

	bool scope::contains_named_value(const std::string& name) const	{
		// check in the current scopes
		if (m_named_values.contains(name)) {
			return true;
		}

		// if we have a parent scope check in there as well
		if (m_parent != nullptr) {
			return m_parent->contains_named_value(name);
		}

		// variable does not exist in the current scope
		return false;
	}

	std::pair<std::unordered_map<std::string, value_ptr>::iterator, bool> scope::add_named_value(
		const std::string& name, 
		value_ptr value
	) {
		// check parent scopes
		if (contains_named_value(name)) {
			return { {}, false };
		}

		return m_named_values.insert({ name, value });
	}
}