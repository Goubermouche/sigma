#pragma once

namespace channel {
    // functions
    class function_node;
    class function_call_node;

    // variables
    class assignment_node;
    class variable_node;
    class local_declaration_node;
    class global_declaration_node;
    class allocation_node;

    // flow control
    class return_node;

    // types
    // signed integers
    class i8_node;
    class i16_node;
    class i32_node;
    class i64_node;
    // unsigned integers
    class u8_node;
    class u16_node;
    class u32_node;
    class u64_node;
    // floating point
    class f32_node;
    class f64_node;

    // operators
    class operator_addition_node;
    class operator_subtraction_node;
    class operator_multiplication_node;
    class operator_division_node;
    class operator_modulo_node;
}

#include "../llvm_wrappers/value.h"
#include "../llvm_wrappers/function.h"

namespace channel {
    /**
     * \brief Codegen visitor that walks the AST and generates LLVM IR.
     */
    class visitor {
    public:
        virtual ~visitor() = default;

        // functions
        virtual bool visit_function_node(function_node& node, value*& out_value) = 0;
        virtual bool visit_function_call_node(function_call_node& node, value*& out_value) = 0;

        // variables
        virtual bool visit_assignment_node(assignment_node& node, value*& out_value) = 0;
        virtual bool visit_variable_node(variable_node& node, value*& out_value) = 0;
        virtual bool visit_local_declaration_node(local_declaration_node& node, value*& out_value) = 0;
        virtual bool visit_global_declaration_node(global_declaration_node& node, value*& out_value) = 0;
        virtual bool visit_allocation_node(allocation_node& node, value*& out_value) = 0;

        // flow control
        virtual bool visit_return_node(return_node& node, value*& out_value) = 0;

        // types
        // signed integers
        virtual bool visit_keyword_i8_node(i8_node& node, value*& out_value) = 0;
        virtual bool visit_keyword_i16_node(i16_node& node, value*& out_value) = 0;
        virtual bool visit_keyword_i32_node(i32_node& node, value*& out_value) = 0;
        virtual bool visit_keyword_i64_node(i64_node& node, value*& out_value) = 0;
        // unsigned integers
        virtual bool visit_keyword_u8_node(u8_node& node, value*& out_value) = 0;
        virtual bool visit_keyword_u16_node(u16_node& node, value*& out_value) = 0;
        virtual bool visit_keyword_u32_node(u32_node& node, value*& out_value) = 0;
        virtual bool visit_keyword_u64_node(u64_node& node, value*& out_value) = 0;
        // floating point
        virtual bool visit_keyword_f32_node(f32_node& node, value*& out_value) = 0;
        virtual bool visit_keyword_f64_node(f64_node& node, value*& out_value) = 0;

        // operators
        virtual bool visit_operator_addition_node(operator_addition_node& node, value*& out_value) = 0;
        virtual bool visit_operator_subtraction_node(operator_subtraction_node& node, value*& out_value) = 0;
        virtual bool visit_operator_multiplication_node(operator_multiplication_node& node, value*& out_value) = 0;
        virtual bool visit_operator_division_node(operator_division_node& node, value*& out_value) = 0;
        virtual bool visit_operator_modulo_node(operator_modulo_node& node, value*& out_value) = 0;
    };
}