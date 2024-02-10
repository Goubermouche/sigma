#include "scope.h"

namespace sigma {
	scope::scope(scope_type type) : scope_ty(type) {}

	auto scope::find_parent_namespace() const -> handle<namespace_scope> {
		if (parent->scope_ty == scope_type::NAMESPACE) {
			return parent;
		}

		return parent->find_parent_namespace();
	}

	auto scope::find_variable(const utility::string_table_key& identifier) -> handle<variable> {
		const auto it = variables.find(identifier);
		if(it != variables.end()) {
			return &it->second;
		}

		if(parent) {
			return parent->find_variable(identifier);
		}

		return nullptr;
	}

	auto scope::find_type(const utility::string_table_key& identifier) -> handle<type> {
		const auto it = types.find(identifier);
		if (it != types.end()) {
			return &it->second;
		}

		if(parent) {
			return parent->find_type(identifier);
		}

		return nullptr;
	}

	namespace_scope::namespace_scope(scope_type type) : scope(type) {}

	auto namespace_scope::find_namespace(const namespace_list& namespaces, u64 index) -> handle<scope> {
		if (index == namespaces.size()) {
			return this;
		}

		// we still have namespace directives, traverse into those scopes
		const auto it = child_namespaces.find(namespaces[index]);
		if (it != child_namespaces.end()) {
			return it->second->find_namespace(namespaces, index + 1);
		}

		return nullptr;
	}
} // namespace sigma 
