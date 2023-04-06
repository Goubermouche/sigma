#include "scope.h"

namespace channel {
	scope::scope(scope* parent)
		: m_parent(parent) {}

	void scope::insert_named_value(const std::string& name, llvm::Value* value) {
		m_named_values[name] = value;
	}

	llvm::Value* scope::get_named_value(const std::string& name) {
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

	bool scope::contains_named_value(const std::string& name) const	{
		// check in the current scopes
		if (m_named_values.contains(name)) {
			return true;
		}

		// if we have a parent scope check in there as well
		if (m_parent != nullptr) {
			return m_parent->contains_named_value(name);
		}

		// value does not exist in the current scope
		return false;
	}

	std::pair<std::unordered_map<std::string, llvm::Value*>::iterator, bool> scope::add_named_value(const std::string& name, llvm::Value* value) {
		// check parent scopes
		if (contains_named_value(name)) {
			return { {}, false };
		}

		return m_named_values.insert({ name, value });
	}

	std::unique_ptr<scope> scope::create_nested_scope(const std::string& name) {
		std::cout << "creating nested scope\n";
		return std::make_unique<scope>(this);
	}
}