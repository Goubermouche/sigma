#pragma once
#include "llvm_wrappers/type.h"
#include "llvm_wrappers/functions/function.h"
#include "llvm_wrappers/llvm_context.h"

namespace sigma {
	using function_ptr = std::shared_ptr<function>;
	using function_declaration_ptr = std::shared_ptr<function_declaration>;

	class function_registry {
	public:
		function_registry() = default;

		function_ptr get_function(
			const std::string& identifier,
			const std::shared_ptr<llvm_context>& context
		);

		function_declaration_ptr get_function_declaration(
			const std::string& identifier
		) const;

		function_declaration_ptr get_external_function_declaration(
			const std::string& identifier
		) const;

		const std::unordered_map<std::string, function_declaration_ptr>& get_external_function_declarations() const;

		void insert_function(
			const std::string& identifier,
			function_ptr function
		);

		void insert_function_declaration(
			const std::string& identifier,
			function_declaration_ptr function
		);

		bool contains_function(
			const std::string& identifier
		) const;

		bool contains_function_declaration(
			const std::string& identifier
		) const;
	private:
		// definitions
		std::unordered_map<std::string, function_ptr> m_functions;

		// declarations
		std::unordered_map<std::string, function_declaration_ptr> m_function_declarations;
		std::unordered_map<std::string, function_declaration_ptr> m_external_function_declarations = {{
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
	};
}