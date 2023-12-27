#pragma once
#include <abstract_syntax_tree/abstract_syntax_tree.h>
#include <intermediate_representation/builder.h>
#include <utility/containers/string_table.h>

namespace sigma {
	struct backend_context;

	class variable_registry {
	public:
		struct variable {
			enum variable_flags {
				NONE = 0,

				// variable is a function parameter, function parameters don't need to be loaded
				FUNCTION_PARAMETER = 1 
			};

			handle<ir::node> value;
			variable_flags flags;
			data_type type;
		};

		struct scope {
			auto get_variable(utility::string_table_key identifier) -> handle<variable>;
			bool contains(utility::string_table_key identifier) const;

			handle<scope> parent = nullptr;
			std::vector<scope> children;

			std::unordered_map<utility::string_table_key, variable> variables;
			u16 trace_index = 0;
		};

		variable_registry(backend_context& context);

		// NOTE: since we have two step analysis we need to traverse the scope system twice - first,
		//       we construct it using push_scope() and pop_scope() functions, and then we retrace it
		//       using the trace_push_scope() and trace_pop_scope() functions

		void push_scope();
		void pop_scope();

		void trace_push_scope();
		void trace_pop_scope();

		void reset_active_scope();

		void pre_declare_variable(utility::string_table_key identifier, data_type type) const;
		auto declare_variable(utility::string_table_key identifier, u16 size, u16 alignment) const -> handle<ir::node>;

		/**
		 * \brief Attempts to retrieve the variable declared under the specified \b identifier.
		 * \param identifier Variable identifier to look for
		 * \return handle<variable> if a valid variable is found, nullptr otherwise.
		 */
		auto get_variable(utility::string_table_key identifier) const -> handle<variable>;
		bool contains(utility::string_table_key identifier) const;

		auto create_load(utility::string_table_key identifier, ir::data_type type, u16 alignment) const -> handle<ir::node>;
		void create_store(utility::string_table_key identifier, handle<ir::node> value, u16 alignment) const;
	private:
		backend_context& m_context;

		// local variables
		scope m_global_scope;
		handle<scope> m_active_scope;
	};

	FLAG_ENUM(variable_registry::variable::variable_flags);
} // namespace sigma
