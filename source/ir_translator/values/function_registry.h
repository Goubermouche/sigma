#pragma once
#include <intermediate_representation/builder.h>
#include <utility/containers/symbol_table.h>

namespace sigma::detail {
	using namespace utility::types;

	struct external_function {
		handle<ir::external> external;
		ir::function_signature signature;
	};

	class function_registry {
	public:
		function_registry(ir::builder& builder);

		/**
		 * \brief Declares a new function.
		 * \param identifier_key Unique identifier key
		 * \param function_sig Function to declare
		 */
		void register_function(
			utility::symbol_table_key identifier_key, const ir::function_signature& function_sig
		);

		/**
		 * \brief Declares a new external function.
		 * \param identifier_key Unique identifier key
		 * \param function_sig Function signature of the external function
		 */
		void register_external_function(
			utility::symbol_table_key identifier_key, const ir::function_signature& function_sig
		);
		
		/**
		 * \brief Creates a call to the function located under the specified \b identifier \b key.
		 * \param identifier_key Callee identifier key
		 * \param parameters List of parameters to call the function with
		 * \return Callee return value.
		 */
		[[nodiscard]] auto create_call(
			utility::symbol_table_key identifier_key, const std::vector<handle<ir::node>>& parameters
		) -> handle<ir::node>;
	private:
		ir::builder& m_builder;

		// TODO: the key should take parameter data types into account
		std::unordered_map<utility::symbol_table_key, handle<ir::function>> m_functions;
		std::unordered_map<utility::symbol_table_key, external_function> m_external_functions;
	};
} // sigma::detail
