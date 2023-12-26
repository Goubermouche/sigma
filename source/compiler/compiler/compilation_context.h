#pragma once
#include "compiler/compiler/type_system/function_registry.h"
#include "compiler/compiler/type_system/variable_registry.h"

#include <intermediate_representation/builder.h>
#include <utility/containers/string_table.h>
#include <tokenizer/token_buffer.h>

// TODO: add support for emitting to .dot files

namespace sigma {
	struct syntax {
		void print_ast() const;

		utility::string_table string_table;
		abstract_syntax_tree ast;
	};

	struct frontend_context {
		void print_tokens() const;

		syntax syntax;
		token_buffer tokens;
	};
	
	struct backend_context {
		backend_context(syntax syntax, ir::target target);

		syntax syntax;

		// ir stuff
		ir::module module;
		ir::builder builder;

		// analysis
		function_registry function_registry;
		variable_registry variable_registry;
	};
} // namespace sigma
