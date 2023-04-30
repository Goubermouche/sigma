#include "access_node.h"

namespace channel {
    access_node::access_node(u64 line_number, const std::string& variable_identifier)
	    :node(line_number), m_variable_identifier(variable_identifier) {}

    bool access_node::accept(visitor& visitor, value*& out_value) {
        LOG_NODE_NAME(variable_node);
        return visitor.visit_access_node(*this, out_value);
    }

    void access_node::print(int depth, const std::wstring& prefix, bool is_last) {
        print_value(depth, prefix, L"VariableAccess", "'" + m_variable_identifier +"'\n", is_last);
    }

    const std::string& access_node::get_variable_identifier() const {
        return m_variable_identifier;
    }
}