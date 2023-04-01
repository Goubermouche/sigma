#include "variable_node.h"

namespace channel {
    variable_node::variable_node(const std::string& name)
	    : m_name(name){}

    llvm::Value* variable_node::accept(visitor& visitor) {
        return visitor.visit_variable_node(*this);
    }

    const std::string& variable_node::get_name() const {
        return m_name;
    }
}

