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

	outcome::result<void> scope::concatenate(
		const scope_ptr& other
	) {
		if (!other) {
			return outcome::success();
		}

		for (const auto& variable : other->m_variables) {
			if (m_variables.contains(variable.first)) {
				return outcome::failure(
					error::emit<4005>(
						variable.first
					)
				);
			}

			m_variables.insert(variable);
		}

		if (other->m_loop_end_block && !m_loop_end_block) {
			m_loop_end_block = other->m_loop_end_block;
		}

		// Merge all child scopes
		for (const auto& child : other->m_children) {
			// Create a new scope with the same parent and merge the child into it.
			auto new_child = std::make_shared<scope>(m_parent);
			OUTCOME_TRY(new_child->concatenate(child));
			m_children.push_back(new_child);
		}

		return outcome::success();
	}

	void scope::add_child(
		const scope_ptr child
	) {
		m_children.push_back(child);
	}

	variable_ptr scope::get_variable(
		const std::string& identifier
	) {
		// try to find an llvm::Value in this scope
		const auto it = m_variables.find(identifier);
		if(it != m_variables.end()) {
			return it->second;
		}

		// if we haven't found a value, but we have a parent scope we need to search it as well 
		if (m_parent != nullptr) {
			return m_parent->get_variable(identifier);
		}

		// value does not exist in current scope
		return nullptr;
	}

	bool scope::contains_variable(
		const std::string& identifier
	) const	{
		// check in the current scopes
		if (m_variables.contains(identifier)) {
			return true;
		}

		// if we have a parent scope check in there as well
		if (m_parent != nullptr) {
			return m_parent->contains_variable(identifier);
		}

		// variable does not exist in the current scope
		return false;
	}

	bool scope::insert_variable(
		const std::string& identifier,
		variable_ptr variable
	) {
		return m_variables.insert({ identifier, variable }).second;
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

	void scope::print(u64 level) const {
		const u64 indent = level * 2;

		console::out
			<< std::string(indent, ' ')
			<< color::orange
			<< "scope (level " << level << ")\n"
			<< color::white;

		for (const auto& [identifier, variable] : m_variables) {
			console::out
				<< std::string(indent, ' ')
				<< identifier << ": "
				<< variable->get_value()->get_type().to_string() << '\n';
		}

		for(const auto& child : m_children) {
			child->print(level + 1);
		}
	}
}