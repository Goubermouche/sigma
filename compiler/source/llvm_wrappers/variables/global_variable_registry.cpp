#include "global_variable_registry.h"

namespace sigma {
    u64 global_variable_registry::get_global_ctors_count() const {
        return m_global_ctors.size();
    }

    const std::vector<llvm::Constant*>& global_variable_registry::get_global_ctors() const {
        return m_global_ctors;
    }

    void global_variable_registry::add_global_ctor(llvm::Constant* ctor) {
        m_global_ctors.push_back(ctor);
    }

    u64 global_variable_registry::get_initialization_priority() const {
        return m_initialization_priority;
    }

    void global_variable_registry::increment_initialization_priority() {
        m_initialization_priority++;
    }

    bool global_variable_registry::insert(
        const std::string& variable_name, value_ptr value
    ) {
        return m_variables.insert(variable_name, value);
    }

    bool global_variable_registry::contains(
        const std::string& variable_name
    ) const {
        return m_variables.contains(variable_name);
    }

    value_ptr global_variable_registry::get(
        const std::string& variable_name
    ) {
        return m_variables.get(variable_name);
    }
}