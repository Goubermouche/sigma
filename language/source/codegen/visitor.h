#pragma once

namespace channel {
    // keywords
    class keyword_i8_node;
    class keyword_i16_node;
    class keyword_i32_node;
    class keyword_i64_node;

    class keyword_u8_node;
    class keyword_u16_node;
    class keyword_u32_node;
    class keyword_u64_node;

    // operators
    class operator_addition_node;
    class operator_subtraction_node;
    class operator_multiplication_node;
    class operator_division_node;
    class operator_modulo_node;
    class return_node;

    // variables
    class function_call_node;
    class variable_node;
    class assignment_node;
    class function_node;

    class local_declaration_node;
    class global_declaration_node;
}

#include <llvm/IR/Value.h>

namespace channel {
    /**
     * \brief Codegen visitor that walks the AST and generates LLVM IR.
     */
    class visitor {
    public:
        virtual ~visitor() = default;

        virtual llvm::Value* visit_local_declaration_node(local_declaration_node& node) = 0;
        virtual llvm::Value* visit_global_declaration_node(global_declaration_node& node) = 0;

		virtual llvm::Value* visit_assignment_node(assignment_node& node) = 0;
        virtual llvm::Value* visit_function_call_node(function_call_node& node) = 0;
        virtual llvm::Value* visit_variable_node(variable_node& node) = 0;
        virtual llvm::Value* visit_function_node(function_node& node) = 0;
        virtual llvm::Value* visit_return_node(return_node& node) = 0;

        // keywords
        virtual llvm::Value* visit_keyword_i8_node(keyword_i8_node& node) = 0;
        virtual llvm::Value* visit_keyword_i16_node(keyword_i16_node& node) = 0;
        virtual llvm::Value* visit_keyword_i32_node(keyword_i32_node& node) = 0;
        virtual llvm::Value* visit_keyword_i64_node(keyword_i64_node& node) = 0;

        virtual llvm::Value* visit_keyword_u8_node(keyword_u8_node& node) = 0;
        virtual llvm::Value* visit_keyword_u16_node(keyword_u16_node& node) = 0;
        virtual llvm::Value* visit_keyword_u32_node(keyword_u32_node& node) = 0;
        virtual llvm::Value* visit_keyword_u64_node(keyword_u64_node& node) = 0;

        // operators
        virtual llvm::Value* visit_operator_addition_node(operator_addition_node& node) = 0;
        virtual llvm::Value* visit_operator_subtraction_node(operator_subtraction_node& node) = 0;
        virtual llvm::Value* visit_operator_multiplication_node(operator_multiplication_node& node) = 0;
        virtual llvm::Value* visit_operator_division_node(operator_division_node& node) = 0;
        virtual llvm::Value* visit_operator_modulo_node(operator_modulo_node& node) = 0;
    };
}