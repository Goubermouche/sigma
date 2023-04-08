#include "node.h"
#include "../visitor.h"

namespace channel {
    node::node(u64 line_index)
	    : m_line_index(line_index) {}

    u64 node::get_declaration_line_index() const {
        return m_line_index;
    }

    //void node::set_declaration_line_index(u64 line_index) {
    //    m_line_index = line_index;
    //}
}