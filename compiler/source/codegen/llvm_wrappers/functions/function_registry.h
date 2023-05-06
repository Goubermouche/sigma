#pragma once
#include "function.h"

namespace channel {
	using function_ptr = std::shared_ptr<function>;

	class function_registry {
	public:
		function_registry() = default;

		function_ptr get_function(const std::string& identifier) const;
		void insert_function(const std::string& identifier, function_ptr function);
		bool contains_function(const std::string& identifier) const;
	private:
		std::unordered_map<std::string, function_ptr> m_functions;
	};
}