#include "variable_registry.h"

namespace sigma {
	variable_registry::variable_registry() {
		m_scopes.push_back(std::make_shared<scope>(nullptr));
	}

	bool variable_registry::insert_local_variable(
		const std::string& identifier, 
		variable_ptr variable
	) {
		return m_scopes.back()->insert_variable(identifier, variable);
	}

	bool variable_registry::insert_global_variable(
		const std::string& identifier,
		variable_ptr variable
	) {
		return m_global_variables.insert({ identifier, variable }).second;
	}

	variable_ptr variable_registry::get_variable(
		const std::string& identifier
	) {
		if(const auto& value = get_global_variable(identifier)) {
			return value;
		}

		return get_local_variable(identifier);
	}

	variable_ptr variable_registry::get_local_variable(
		const std::string& identifier
	) {
		return m_scopes.back()->get_variable(identifier);
	}

	variable_ptr variable_registry::get_global_variable(
		const std::string& identifier
	) {
		const auto it = m_global_variables.find(identifier);
		if(it == m_global_variables.end()) {
			return nullptr;
		}

		return it->second;
	}

	bool variable_registry::contains_variable(
		const std::string& identifier
	) {
		if(m_scopes.back()->contains_variable(identifier)) {
			return true;
		}

		return m_global_variables.contains(identifier);
	}

	bool variable_registry::contains_global_variable(
		const std::string& identifier
	) {
		return m_global_variables.contains(identifier);
	}

	void variable_registry::push_scope() {
		auto new_scope = std::make_shared<scope>(m_scopes.back());
		m_scopes.back()->add_child(new_scope);
		m_scopes.push_back(new_scope);
	}

	void variable_registry::pop_scope() {
		if (m_scopes.size() > 1) {
			m_scopes.pop_back();
		}
	}

	outcome::result<void> variable_registry::concatenate(
		const variable_registry& other
	) {
		// local variables
		for (size_t i = 0; i < other.m_scopes.size(); ++i) {
			// if the current registry doesn't have enough scopes, create new ones
			if (i >= m_scopes.size()) {
				m_scopes.push_back(std::make_shared<scope>(m_scopes.back()));
			}

			// merge the scopes
			OUTCOME_TRY(m_scopes[i]->concatenate(other.m_scopes[i]));
		}

		// global variables
		for (const auto& variable : other.m_global_variables) {
			if (m_global_variables.contains(variable.first)) {
				// return outcome::failure(
				// 	error::emit<error_code::global_variable_already_defined>(
				// 		file_range{}, //variable.second->get_position(),
				// 		variable.first,
				// 		m_global_variables[variable.first]->get_position()
				// 	)
				// );
			}

			m_global_variables.insert(variable);
		}

		return outcome::success();
	}

	llvm::BasicBlock* variable_registry::get_loop_end_block() const {
		return m_scopes.back()->get_loop_end_block();
	}

	void variable_registry::print() const {
		console::out
			<< color::red
			<< "variable registry:\n"
			<< color::white;

		console::out
			<< color::yellow
			<< "local variables:\n"
			<< color::white;

		m_scopes[0]->print(0);

		console::out
			<< color::yellow
			<< "global variables:\n"
			<< color::white;

		for(const auto& [identifier, variable] : m_global_variables) {
			console::out
				<< std::string(2, ' ')
				<< identifier << ": "
				<< variable->get_value()->get_type().to_string() << '\n';
		}
	}

	u64 variable_registry::get_global_variable_count() const {
		return m_global_variables.size();
	}
}