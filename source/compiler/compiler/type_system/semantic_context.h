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
	} // namespace detail 
	
	class semantic_context {
	public:
		semantic_context(backend_context& context);

		/**
		 * \brief Pushes a new scope.
		 */
		void push_scope();

		/**
		 * \brief Pops the current scope/namespace.
		 */
		void pop_scope();

		/**
		 * \brief Pushes a new namespace.
		 * \param name Name of the new namespaces
		 */
		void push_namespace(utility::string_table_key name);

		/**
		 * \brief Resets the active scope to the global namespace.
		 */
		void reset_active_scope();

		/**
		 * \brief 'Fake pushes' a scope/namespace according to m_trace. Used for re-traversing the scope tree
		 * when translating to our IR.
		 */
		void trace_push_scope();

		/**
		 * \brief Pops the current scope/namespace.
		 */
		void trace_pop_scope();

		auto pre_declare_variable(utility::string_table_key identifier, data_type type) const -> variable&;
		void pre_declare_local_function(const function_signature& signature) const;

		auto declare_variable(utility::string_table_key identifier, u16 size, u16 alignment) const -> handle<ir::node>;
		void declare_external_function(const function_signature& signature) const;
		void declare_local_function(const function_signature& signature) const;
		void declare_implicit_return() const;

		auto create_load(utility::string_table_key identifier, ir::data_type type, u16 alignment) const -> handle<ir::node>;
		void create_store(utility::string_table_key identifier, handle<ir::node> value, u16 alignment) const;

		auto create_call(const function_signature& callee_signature, const std::vector<utility::string_table_key>& namespaces, const std::vector<handle<ir::node>>& parameters) const -> handle<ir::node>;
		auto create_callee_signature(handle<node> function_node, const std::vector<data_type>& parameter_types) -> utility::result<function_signature>;

		/**
		 * \brief Attempts to locate a variable given an \b identifier and a list of \b namespaces.
		 * \param identifier Identifier key to the variable name
		 * \param namespaces List of namespace directives (ie. utility::detail would be { utility, detail }.
		 * \return result<handle<variable>>, \b note \b that \b the \b handle \b itself \b can \b still \b be \b null, in which case the variable does not exist.
		 */
		auto find_variable(utility::string_table_key identifier, const std::vector<utility::string_table_key>& namespaces = {}) const -> utility::result<handle<variable>>;

		/**
		 * \brief \b TEMPORARY. Retrieves the variable given an \b identifier and a list of \b namespaces. This function will be replaced when full
		 * namespace support is implemented
		 * \param identifier identifier Identifier key to the variable name
		 * \param namespaces namespaces List of namespace directives (ie. utility::detail would be { utility, detail }.
		 * \return handle<variable>, \b note \b that \b the \b handle \b itself \b can \b still \b be \b null, in which case the variable does not exist.
		 */
		auto get_variable(utility::string_table_key identifier, const std::vector<utility::string_table_key>& namespaces = {}) const -> handle<variable>;

		bool contains_variable(utility::string_table_key identifier) const;
		bool contains_function(const function_signature& signature) const;
	private:
		/**
		 * \brief Locate the namespace specified by \b namespaces. Relative to the current scope.
		 * \param namespaces Namespaces to apply to the current scope
		 * \return handle<scope> / nullptr.
		 */
		auto find_namespace(const std::vector<utility::string_table_key>& namespaces) const -> handle<scope>;

		/**
		 * \brief Locate the first parent namespace of the current scope.
		 * \return handle<namespace_scope> (guaranteed).
		 */
		auto find_parent_namespace() const-> handle<namespace_scope>;

		auto find_relative_namespace(const std::vector<utility::string_table_key>& namespaces) const -> handle<namespace_scope>;

		auto allocate_scope() const -> handle<scope>;
		auto allocate_namespace() const -> handle<scope>;

		auto construct_namespace_chain(const std::vector<utility::string_table_key>& namespaces) const ->std::stringstream;

		auto emit_no_viable_overload_error(handle<node> function_node) -> utility::error;
		auto emit_unknown_namespace_error(const std::vector<utility::string_table_key>& namespaces) const-> utility::error;
	private:
		backend_context& m_context;

		handle<namespace_scope> m_global_scope;
		handle<scope> m_current_scope;

		// store a list of all pushes for traversing the scope tree later
		std::vector<handle<scope>> m_trace;
		u64 m_trace_index = 0;
	};
} // namespace sigma
