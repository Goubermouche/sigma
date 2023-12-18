#pragma once
#include <intermediate_representation/builder.h>
#include <utility/containers/string_table.h>

namespace sigma::detail {
	using namespace utility::types;

	class variable_registry {
	public:
		variable_registry(sigma::ir::builder& builder);
		
		/**
		 * \brief Declares a new local variable.
		 * \param identifier_key Unique identifier key
		 * \param size Size of the variable [bytes]
		 * \param alignment Alignment of the variable [bytes]
		 * \return Handle<ir::node> representing the local variable.
		 */
		auto register_variable(
			utility::string_table_key identifier_key, u16 size, u16 alignment
		) -> handle<ir::node>;
		
		/**
		 * \brief Loads the variable located under the specified \b identifier \b key.
		 * \param identifier_key Unique identifier key
		 * \param type Type to interpret the variable as
		 * \param alignment Alignment of the load operation [bytes]
		 * \return Handle<ir::node> representing the loaded variable.
		 */
		auto create_load(
			utility::string_table_key identifier_key, ir::data_type type, u16 alignment
		) -> handle<ir::node>;

		/**
		 * \brief Stores the specified \b value in the given variable.
		 * \param identifier_key Unique identifier key of the target variable
		 * \param value Value to store
		 * \param alignment Alignment of the store operation [bytes]
		 */
		void create_store(
			utility::string_table_key identifier_key, handle<ir::node> value, u16 alignment
		);
	private:
		ir::builder& m_builder;

		std::unordered_map<utility::string_table_key, handle<ir::node>> m_variables;
	};
} // namespace sigma::detail

