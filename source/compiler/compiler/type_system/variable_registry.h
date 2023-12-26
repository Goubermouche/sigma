#pragma once
#include <abstract_syntax_tree/abstract_syntax_tree.h>
#include <intermediate_representation/builder.h>
#include <utility/containers/string_table.h>

namespace sigma {
	struct backend_context;

	class variable_registry {
	public:
		struct variable {
			handle<ir::node> value;
			data_type type;
		};

		variable_registry(backend_context& context);

		void pre_declare_variable(utility::string_table_key identifier, data_type type);
		auto declare_variable(utility::string_table_key identifier, u16 size, u16 alignment) -> handle<ir::node>;

		/**
		 * \brief Attempts to retrieve the variable declared under the specified \b identifier.
		 * \param identifier Variable identifier to look for
		 * \return handle<variable> if a valid variable is found, nullptr otherwise.
		 */
		auto get_variable(utility::string_table_key identifier) -> handle<variable>;

		auto create_load(utility::string_table_key identifier, ir::data_type type, u16 alignment) -> handle<ir::node>;
		void create_store(utility::string_table_key identifier, handle<ir::node> value, u16 alignment);
	private:
		backend_context& m_context;

		// TODO: use a scope-based system
		std::unordered_map<utility::string_table_key, variable> m_variables;
	};
} // namespace sigma
