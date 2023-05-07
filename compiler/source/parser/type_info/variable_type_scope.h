#pragma once
#include "codegen/llvm_wrappers/type.h"

namespace channel {
	class variable_type_scope;
	using variable_type_scope_ptr = std::shared_ptr<variable_type_scope>;

	class variable_type_scope {
	public:
		variable_type_scope(
			variable_type_scope_ptr parent
		);

		void insert_variable_type(const std::string& identifier, type type);

		bool contains_variable_type(const std::string& identifier) const;
		type get_variable_type(const std::string& identifier) const;
	private:
		variable_type_scope_ptr m_parent;
		std::unordered_map<std::string, type> m_variable_types;
	};
}