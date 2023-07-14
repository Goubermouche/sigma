#pragma once
#include "llvm_wrappers/value.h"

namespace sigma {
	class variable_registry {
	public:
		bool insert(
			const std::string& variable_name,
			value_ptr value
		);

		bool contains(
			const std::string& variable_name
		) const;

		value_ptr get(
			const std::string& variable_name
		);
	private:
		std::unordered_map<std::string, value_ptr> m_variables;
	};
}
