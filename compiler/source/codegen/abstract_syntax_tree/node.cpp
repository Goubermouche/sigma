#include "node.h"
#include "../visitor/visitor.h"

namespace channel {
    node::node(u64 line_number)
	    : m_line_number(line_number) {}

    u64 node::get_declaration_line_number() const {
        return m_line_number;
    }
}