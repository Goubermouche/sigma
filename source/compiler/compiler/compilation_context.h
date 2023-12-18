#pragma once
#include <abstract_syntax_tree/abstract_syntax_tree.h>
#include <utility/containers/string_table.h>
#include <tokenizer/token_buffer.h>

namespace sigma {
	struct compilation_context {
		void print_tokens() const;
		void print_ast() const;

		utility::string_table strings;
		abstract_syntax_tree ast;
		token_buffer tokens;
	};
} // namespace sigma
