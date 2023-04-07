#pragma once
#include "../utility/macros.h"
#include "../type.h"

namespace channel {
	class value {
	public:
		value(type type, llvm::Value* value);
		type get_type() const;
		llvm::Value* get_value() const;
	private:
		type m_type;
		llvm::Value* m_value;
	};

	class scope	{
	public:
		scope(scope* parent = nullptr);

		void insert_named_value(const std::string& name, llvm::Value* value);
		llvm::Value* get_named_value(const std::string& name);
		bool contains_named_value(const std::string& name) const;
		std::pair<std::unordered_map<std::string, llvm::Value*>::iterator, bool> add_named_value(const std::string& name, llvm::Value* value);

		std::unique_ptr<scope> create_nested_scope(const std::string& name);
	private:
		scope* m_parent = nullptr;
		std::unordered_map<std::string, llvm::Value*> m_named_values;
		std::unordered_map<std::string, llvm::Function*> m_functions;
	};
}
