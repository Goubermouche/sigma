#include "variable_node.h"

namespace channel {
    variable_node::variable_node(const std::string& name)
	    : m_name(name){}

    value* variable_node::accept(visitor& visitor) {
        LOG_NODE_NAME(variable_node);
        return visitor.visit_variable_node(*this);
    }

    std::string variable_node::get_node_name() const {
        return "variable_node";
    }

    const std::string& variable_node::get_name() const {
        return m_name;
    }
}

