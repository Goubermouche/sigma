#pragma once
#include <intermediate_representation/builder.h>
#include <utility/containers/string_table.h>
#include <abstract_syntax_tree/node.h>
#include <parser/data_type.h>

namespace sigma {
	using namespace utility::types;

	struct variable {
		enum variable_flags {
			NONE = 0,
			FUNCTION_PARAMETER = 1,
			LOCAL = 2,
			GLOBAL = 4
		};

		handle<ir::node> value;
		variable_flags flags;
		data_type type;
	};

	FLAG_ENUM(variable::variable_flags);

	struct external_function {
		handle<ir::external> ir_function;
		ir::function_signature ir_signature;
	};

	struct namespace_scope;

	struct scope {
		enum class scope_type : u8 {
			NONE = 0,
			REGULAR,
			NAMESPACE
		};

		enum class control_type : u8 {
			NONE = 0,
			CONDITIONAL,
			UNCONDITIONAL
		};

		scope() = default;
		scope(scope_type type);

		auto find_parent_namespace() const -> handle<namespace_scope>;
		auto find_variable(const utility::string_table_key& identifier) -> handle<variable>;

		std::unordered_map<utility::string_table_key, variable> variables;
		// TODO: types

		handle<scope> parent = nullptr;
		std::vector<handle<scope>> child_scopes;

		// metadata
		scope_type type = scope_type::NONE;
		control_type control = control_type::NONE;
		bool has_return = false;
	};

	/**
	 * \brief Scope representing a namespace. 
	 */
	struct namespace_scope : scope {
		namespace_scope(scope_type type);

		auto find_namespace(const std::vector<utility::string_table_key>& namespaces, u64 index) -> handle<scope>;

		// NOTE: we're using std::map instead of std::unordered_map because we need deterministic order
		//       of elements (example: we have function A and function B, both of these have the same
		//       cast cost, but the same one should be returned every time, not at random)

		// identifier -> function signature
		std::unordered_map<utility::string_table_key, std::map<function_signature, handle<ir::function>>> local_functions;
		std::unordered_map<utility::string_table_key, std::map<function_signature, external_function>> external_functions;

		// contained namespaces, regular scopes cannot contain namespaces
		std::unordered_map<utility::string_table_key, handle<namespace_scope>> child_namespaces;
	};
} // namespace sigma 
