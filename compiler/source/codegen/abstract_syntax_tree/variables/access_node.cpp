#include "access_node.h"

namespace channel {
    access_node::access_node(u64 line_number, const std::string& variable_identifier)
	    :node(line_number), m_variable_identifier(variable_identifier) {}

    bool access_node::accept(visitor& visitor, value*& out_value) {
        LOG_NODE_NAME(variable_node);
        return visitor.visit_access_node(*this, out_value);
    }

    std::string access_node::get_node_name() const {
        return "access_node";
    }

    const std::string& access_node::get_variable_identifier() const {
        return m_variable_identifier;
    }
}