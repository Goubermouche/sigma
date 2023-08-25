#pragma once
#include "abstract_syntax_tree/llvm_wrappers/type.h"
#include "abstract_syntax_tree/llvm_wrappers/functions/function.h"

#include <utility/diagnostics/error.h>

namespace sigma {
	using function_ptr = ptr<function>;
	using function_declaration_ptr = ptr<function_declaration>;

	// external function declarations
	static inline const std::unordered_map<std::string, function_declaration_ptr> g_external_function_declarations = {
		{
			"print",
			std::make_shared<function_declaration>(
				type(type::base::i32, 0),
				std::vector<std::pair<std::string, type>>{
					{ "print", type(type::base::character, 1) }
				},
				true,
				"printf"
			)
		}, {
			"printc",
			std::make_shared<function_declaration>(
				type(type::base::empty, 0),
				std::vector<std::pair<std::string, type>>{
					{ "character", type(type::base::character, 0) }
				},
				false,
				"putchar"
			)
		}, {
			"malloc",
			std::make_shared<function_declaration>(
				type(type::base::i8, 1),
				std::vector<std::pair<std::string, type>>{
					{ "size", type(type::base::u64, 0) }
				},
				false,
				"malloc"
			)
		}, {
			"free",
			std::make_shared<function_declaration>(
				type(type::base::empty, 0),
				std::vector<std::pair<std::string, type>>{
					{ "ptr", type(type::base::empty, 1) }
				},
				false,
				"free"
			)
		}, {
			"memset",
			std::make_shared<function_declaration>(
				type(type::base::empty, 1),
				std::vector<std::pair<std::string, type>>{
					{ "ptr", type(type::base::character, 1)},
					{ "value", type(type::base::i32, 0) },
					{ "num", type(type::base::u64, 0) }
				},
				false,
				"memset"
			)
		}, {
			"sin",
			std::make_shared<function_declaration>(
				type(type::base::f64, 0),
				std::vector<std::pair<std::string, type>>{
					{  "value", type(type::base::f64, 0) }
				},
				false,
				"sin"
			)
		}, {
			"cos",
			std::make_shared<function_declaration>(
				type(type::base::f64, 0),
				std::vector<std::pair<std::string, type>>{
					{  "value", type(type::base::f64, 0) }
				},
				false,
				"cos"
			)
		}
	};

	class abstract_syntax_tree_context;

	/**
	 * \brief Function registry, contains a set of various function types for the given compilation unit. 
	 */
	class function_registry {
	public:
		function_registry() = default;

		/**
		 * \brief Gets the function with the given \a identifier. If no function definition is found the
		 * method fall backs on external function declarations. 
		 * \param identifier Function identifier to look for
		 * \param context Context to use when a fall back is required
		 * \return function_ptr if a function is found, nullptr otherwise.
		 */
		function_ptr get_function(
			const std::string& identifier,
			const ptr<abstract_syntax_tree_context>& context
		);

		static function_declaration_ptr get_external_function_declaration(
			const std::string& identifier
		);

		/**
		 * \brief Concatenates \a this and \a other function registries. Does not change the \a other registry,
		 * and only updates the current registry.
		 * \param other Second registry to concatenate with.
		 * \return Outcome (void)
		 */
		utility::outcome::result<void> concatenate(
			const function_registry& other
		);

		/**
		 * \brief Attempts to insert a function definition into the registry. 
		 * \param identifier Identifier to insert the function under
		 * \param function Function to insert
		 * \return True if the insertion operation succeeded, false otherwise.
		 */
		bool insert_function(
			const std::string& identifier,
			function_ptr function
		);

		/**
		 * \brief Checks if the registry contains the given function \a identifier. Checks for function definitions and
		 * external function declarations. 
		 * \param identifier Identifier to query 
		 * \return True if the registry contains the queries \a identifier, false otherwise.
		 */
		bool contains_function(
			const std::string& identifier
		) const;
	private:
		std::unordered_map<std::string, function_ptr> m_functions;
	};
}