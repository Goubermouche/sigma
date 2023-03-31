#pragma once

namespace channel {
    class node;
    class keyword_i32_node;
    class operator_addition_node;
}

#include "abstract_syntax_tree/keywords/types/keyword_i32_node.h"
#include "abstract_syntax_tree/operators/operator_addition_node.h"
#include <llvm/IR/Value.h>

namespace channel {
    /**
     * \brief Codegen visitor that walks the AST and generates LLVM IR.
     */
    class visitor {
    public:
        virtual ~visitor() = default;

        // keywords
        virtual llvm::Value* visit_keyword_i32_node(keyword_i32_node& node) = 0;

        // operators
        virtual llvm::Value* visit_operator_addition_node(operator_addition_node& node) = 0;
    };
}