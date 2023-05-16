#pragma once
#include "llvm_wrappers/value.h"

namespace channel {
	class scope;
	using scope_ptr = std::shared_ptr<scope>;

	class scope	{
	public:
		scope(
			scope_ptr parent,
			llvm::BasicBlock* loop_end_block
		);

		void insert_named_value(const std::string& name, value_ptr value);

		std::pair<std::unordered_map<std::string, value_ptr>::iterator, bool> add_named_value(
			const std::string& name,
			value_ptr value
		);

		bool contains_named_value(const std::string& name) const;

		value_ptr get_named_value(const std::string& name);
		llvm::BasicBlock* get_loop_end_block() const;
	private:
		scope_ptr m_parent = nullptr;
		llvm::BasicBlock* m_loop_end_block = nullptr;
		std::unordered_map<std::string, value_ptr> m_named_values;
	};
}
