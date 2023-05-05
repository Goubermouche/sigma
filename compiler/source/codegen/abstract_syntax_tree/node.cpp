#include "node.h"
#include "../visitor/visitor.h"

namespace channel {
    node::node(const token_position& position)
	    : m_position(position) {}

    const token_position& node::get_declared_position() const {
        return m_position;
    }

    void node::print_value(
        int depth, 
        const std::wstring& prefix,
        const std::string& node_name, 
        bool is_last
    ) {
        console::out
            << prefix
            << (depth == 0 ? L"" : (is_last ? L"╰─" : L"├─"))
            << node_name
            << ' ';
    }

    std::wstring node::get_new_prefix(
        int depth, 
        const std::wstring& prefix,
        bool is_last
    ) {
        return depth == 0 ? L"" : prefix + (is_last ? L"  " : L"│ ");
    }
}
