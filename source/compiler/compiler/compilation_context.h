#pragma once
#include <abstract_syntax_tree/abstract_syntax_tree.h>
#include <utility/containers/symbol_table.h>
#include <tokenizer/token_buffer.h>

namespace sigma {
	struct compilation_context {
		utility::symbol_table symbols;
		abstract_syntax_tree ast;
		token_buffer tokens;
	};
} // namespace sigma