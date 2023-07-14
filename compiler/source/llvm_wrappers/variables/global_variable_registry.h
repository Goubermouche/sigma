#pragma once
#include "llvm_wrappers/variables/variable_registry.h"

namespace sigma {
	class global_variable_registry {
	public:
		u64 get_global_ctors_count() const;
		const std::vector<llvm::Constant*>& get_global_ctors() const;
		void add_global_ctor(llvm::Constant* ctor);

		u64 get_initialization_priority() const;
		void increment_initialization_priority();

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
		variable_registry m_variables;
		std::vector<llvm::Constant*> m_global_ctors;
		u64 m_initialization_priority = 0;
	};
}
