#pragma once
#include "../utility/macros.h"
#include "llvm_wrappers/value.h"

namespace channel {
	class scope	{
	public:
		scope(scope* parent, llvm::BasicBlock* loop_end_block);

		void insert_named_value(const std::string& name, value* value);
		std::pair<std::unordered_map<std::string, value*>::iterator, bool> add_named_value(const std::string& name, value* value);
		bool contains_named_value(const std::string& name) const;

		value* get_named_value(const std::string& name);
		llvm::BasicBlock* get_loop_end_block() const;
	private:
		scope* m_parent = nullptr;
		llvm::BasicBlock* m_loop_end_block = nullptr;
		std::unordered_map<std::string, value*> m_named_values;
	};
}
