#pragma once
#include <abstract_syntax_tree/abstract_syntax_tree.h>
#include <intermediate_representation/builder.h>
#include <utility/containers/string_table.h>

namespace sigma {
	using namespace utility::types;
	struct backend_context;

	auto data_type_to_ir(data_type type) -> ir::data_type;
	auto signature_to_ir(const function_signature& signature, const utility::string_table& string_table) -> ir::function_signature;
	auto mangle_function_identifier(const function_signature& signature, const utility::string_table& string_table) -> std::string;

	class function_registry {
		struct external_function {
			handle<ir::external> ir_function;
			ir::function_signature ir_signature;
		};
	public:
		function_registry(backend_context& context);

		/**
		 * \brief Creates an entry for the specified \b signature without actually initializing it.
		 * \param signature Signature to pre declare 
		 */
		void pre_declare_local_function(const function_signature& signature);

		/**
		 * \brief Declares a new external function using the specified \b signature.
		 * \param signature Signature to declare
		 */
		void declare_external_function(const function_signature& signature);

		/**
		 * \brief Declare a new local function using the specified \b signature. Should only be used on
		 * signatures that were pre-declared using the \b pre_declare_local_function() function.
		 * \param signature Signature to declare
		 */
		void declare_local_function(const function_signature& signature);

		/**
		 * \brief Checks, whether the registry contains a function that matches the given \b signature.
		 * \param signature Signature to look for
		 * \return True if the signature is contained, false otherwise.
		 */
		bool contains_function(const function_signature& signature) const;

		/**
		 * \brief Attempts to find the closest-matching signature using the provided \b identifier and
		 * \b parameter_types. The best match is determined via determining the overall 'cast cost' of
		 * calling the given function with the provided \b parameter_types.
		 * \param identifier Identifier of the function
		 * \param parameter_types Parameter types to match with
		 * \return Closest-matching signature.
		 */
		auto get_callee_signature(utility::string_table_key identifier, const std::vector<data_type>& parameter_types) -> function_signature;
		auto create_call(const function_signature& callee_signature, const std::vector<handle<ir::node>>& parameters) -> handle<ir::node>;
	private:
		static auto calculate_parameter_cast_cost(const function_signature& signature, const std::vector<data_type>& parameter_types) -> u16;
		static auto calculate_cast_cost(const data_type& provided, const data_type& required) -> u16;
	private:
		backend_context& m_context;

		// NOTE: we're using std::map instead of std::unordered_map because we need deterministic order
		//       of elements (example: we have function A and function B, both of these have the same
		//       cast cost, but the same one should be returned every time, not at random)

		// identifier -> function signature
		std::unordered_map<utility::string_table_key, std::map<function_signature, handle<ir::function>>> m_local_functions;
		std::unordered_map<utility::string_table_key, std::map<function_signature, external_function>> m_external_functions;
	};
} // namespace sigma