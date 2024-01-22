#pragma once
#include "compiler/compiler/type_system/semantic_context.h"

#include <abstract_syntax_tree/abstract_syntax_tree.h>
#include <intermediate_representation/builder.h>
#include <utility/containers/string_table.h>
#include <tokenizer/token_buffer.h>

// TODO: add support for emitting to .dot files

namespace sigma {
	struct syntax {
		void print_ast() const;

		utility::string_table strings;
		abstract_syntax_tree ast;
	};

	struct backend_context {
		backend_context(syntax& syntax, ir::target target);

		utility::block_allocator allocator;

		// TEMP: the reference here is just temporary, and will be replaced when we add support for
		//       multiple source files
		syntax& syntax; 
		semantic_context semantics;

		ir::module module;
		ir::builder builder;
	};

	struct frontend_context {
		frontend_context();

		void print_tokens() const;

		utility::block_allocator allocator; // one allocator per file

		token_buffer tokens;                // tokenized representation of the source file
		syntax syntax;                      // ast + strings
	};
} // namespace sigma
