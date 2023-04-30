#pragma once
#include "../utility/macros.h"
#include "llvm_wrappers/value.h"

namespace channel {
	class scope	{
	public:
		scope(scope* parent, llvm::BasicBlock* loop_end_block);

		void insert_named_value(const std::string& name, value_ptr value);
		std::pair<std::unordered_map<std::string, value_ptr>::iterator, bool> add_named_value(const std::string& name, value_ptr value);
		bool contains_named_value(const std::string& name) const;

		value_ptr get_named_value(const std::string& name);
		llvm::BasicBlock* get_loop_end_block() const;
	private:
		scope* m_parent = nullptr;
		llvm::BasicBlock* m_loop_end_block = nullptr;
		std::unordered_map<std::string, value_ptr> m_named_values;
	};
}
