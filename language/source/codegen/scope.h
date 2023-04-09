#pragma once
#include "../utility/macros.h"
#include "llvm_wrappers/value.h"

namespace channel {
	class scope	{
	public:
		scope(scope* parent = nullptr);

		void insert_named_value(const std::string& name, value* value);
		value* get_named_value(const std::string& name);
		bool contains_named_value(const std::string& name) const;
		std::pair<std::unordered_map<std::string, value*>::iterator, bool> add_named_value(const std::string& name, value* value);
	private:
		scope* m_parent = nullptr;
		std::unordered_map<std::string, value*> m_named_values;
	};
}
