#pragma once
#include <utility/containers/string_table.h>

#include "compiler/compiler/type_system/scope.h"

namespace sigma {
	struct backend_context;

	namespace detail {
		auto data_type_to_ir(data_type type) -> ir::data_type;
		auto signature_to_ir(const function_signature& signature, const utility::string_table& string_table) -> ir::function_signature;
		auto mangle_function_identifier(const function_signature& signature, const utility::string_table& string_table) -> std::string;
		auto calculate_parameter_cast_cost(const function_signature& signature, const std::vector<data_type>& parameter_types) -> u16;
		auto calculate_cast_cost(const data_type& provided, const data_type& required) -> u16;

		/**
		 * \brief Determine which cast should be used based on the \b original and \b target type.
		 * \param original Original type (type before the cast)
		 * \param target Target type (type after the cast)
		 * \return True if the cast truncates the value, false otherwise.
		 */
		auto determine_cast_kind(const data_type& original, const data_type& target) -> bool; 
	} // namespace detail 
	
	class semantic_context {
	public:
		semantic_context(backend_context& context);

		auto verify_control_flow(handle<ast::node> function_node) const-> utility::result<void>;

		// scope manipulation
		void push_scope(scope::control_type control);
		void push_namespace(utility::string_table_key name);
		
		void reset_active_scope();
		void pop_scope();

		void trace_push_scope();
		void trace_pop_scope();

		// variables
		auto find_variable(utility::string_table_key identifier, const namespace_list& namespaces = {}) const -> utility::result<handle<variable>>;
		auto get_variable(utility::string_table_key identifier, const namespace_list& namespaces = {}) const -> handle<variable>;
		auto declare_variable(utility::string_table_key identifier, u16 size, u16 alignment) const -> handle<ir::node>;
		auto pre_declare_variable(utility::string_table_key identifier, data_type type) const -> variable&;
		bool contains_variable(utility::string_table_key identifier) const;

		// structs
		auto declare_struct(handle<ast::node> node) const -> utility::result<void>;

		// functions
		auto create_call(const function_signature& callee_signature, const namespace_list& namespaces, const std::vector<handle<ir::node>>& parameters) const -> handle<ir::node>;
		auto find_callee_signature(handle<ast::node> function_node, const std::vector<data_type>& parameter_types) -> utility::result<function_signature>;
		void pre_declare_local_function(const function_signature& signature) const;
		void declare_external_function(const function_signature& signature) const;
		void declare_local_function(const function_signature& signature) const;
		bool contains_function(const function_signature& signature) const;

		// returns
		void define_implicit_return() const;
		void declare_return() const;
		bool has_return() const;

		// memory
		auto create_load(utility::string_table_key identifier, ir::data_type type, u16 alignment) const -> handle<ir::node>;
		void create_store(utility::string_table_key identifier, handle<ir::node> value, u16 alignment) const;
	private:
		// namespaces
		auto find_relative_namespace(const namespace_list& namespaces) const->handle<namespace_scope>;
		auto find_namespace(const namespace_list& namespaces) const -> handle<scope>;
		auto find_parent_namespace() const-> handle<namespace_scope>;

		// errors
		auto emit_no_viable_overload_error(handle<ast::node> function_node) -> utility::error;
		auto emit_unknown_namespace_error(const namespace_list& namespaces) const-> utility::error;

		// misc
		auto allocate_scope() const->handle<scope>;
		auto allocate_namespace() const->handle<scope>;

		static auto all_control_paths_return(handle<scope> scope, handle<ast::node> function_node) -> utility::result<bool>;
	private:
		backend_context& m_context;

		handle<namespace_scope> m_global_scope;
		handle<scope> m_current_scope;

		// store a list of all pushes for traversing the scope tree later
		std::vector<handle<scope>> m_trace;
		u64 m_trace_index = 0;
	};
} // namespace sigma
