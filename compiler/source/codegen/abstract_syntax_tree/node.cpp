#include "node.h"
#include "../visitor/visitor.h"

namespace channel {
    node::node(u64 line_number)
	    : m_line_number(line_number) {}

    u64 node::get_declaration_line_number() const {
        return m_line_number;
    }

    void node::print_value(int depth, const std::wstring& prefix, const std::wstring& node_name, const std::string& value, bool is_last) {
        console::out 
            << prefix
            << (depth == 0 ? L"" : (is_last ? L"╰─" : L"├─"))
            << node_name
            << ' '
            << s_ws_to_s_converter.from_bytes(value);
    }

    std::wstring node::get_new_prefix(int depth, const std::wstring& prefix, bool is_last) {
        return depth == 0 ? L"" : prefix + (is_last ? L"  " : L"│ ");
    }
}
