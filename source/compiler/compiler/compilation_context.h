#pragma once
#include "compiler/compiler/type_system/semantic_context.h"

#include <abstract_syntax_tree/abstract_syntax_tree.h>
#include <intermediate_representation/builder.h>
#include <utility/containers/string_table.h>
#include <tokenizer/token_buffer.h>

// TODO: add support for emitting to .dot files

namespace sigma {
	struct backend_context {
		backend_context(utility::string_table strings, abstract_syntax_tree& ast, ir::target target);

		void print_ast() const;

		utility::block_allocator allocator;
		utility::string_table strings;

		abstract_syntax_tree& ast;

		ir::module module;
		ir::builder builder;

		semantic_context semantics;
	};

	struct frontend_context {
		frontend_context();

		void print_tokens() const;
		void print_ast() const;

		utility::block_allocator allocator; // one allocator per file
		utility::string_table strings;      // all strings used in the relevant file

		abstract_syntax_tree ast;           // AST of the given source file
		token_buffer tokens;                // tokenized representation of the source file
	};
} // namespace sigma
