#include "variable_node.h"

namespace channel {
    variable_node::variable_node(u64 line_number, const std::string& name)
	    :node(line_number), m_name(name) {}

    bool variable_node::accept(visitor& visitor, value*& out_value) {
        LOG_NODE_NAME(variable_node);
        return visitor.visit_variable_node(*this, out_value);
    }

    std::string variable_node::get_node_name() const {
        return "variable_node";
    }

    const std::string& variable_node::get_name() const {
        return m_name;
    }
}

