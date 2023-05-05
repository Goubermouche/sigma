#include "variable_access_node.h"

namespace channel {
    variable_access_node::variable_access_node(
        const token_position& position,
        const std::string& variable_identifier
    ) : node(position),
	m_variable_identifier(variable_identifier) {}

    bool variable_access_node::accept(visitor& visitor, value_ptr& out_value) {
        LOG_NODE_NAME(variable_node);
        return visitor.visit_variable_access_node(*this, out_value);
    }

    void variable_access_node::print(int depth, const std::wstring& prefix, bool is_last) {
        print_value(
            depth, 
            prefix, 
            "variable access", 
            is_last
        );

        console::out
    		<< "'"
    		<< AST_NODE_VARIABLE_COLOR
    		<< m_variable_identifier 
            << color::white
    		<< "'\n";
    }

    const std::string& variable_access_node::get_variable_identifier() const {
        return m_variable_identifier;
    }
}