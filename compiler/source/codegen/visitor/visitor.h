#pragma once
#include "codegen/abstract_syntax_tree/keywords/flow_control/if_else_node.h"

namespace channel {
    // functions
    class function_node;
    class function_call_node;

    // variables
    class assignment_node;
    class access_node;
    class local_declaration_node;
    class global_declaration_node;
    class array_allocation_node;
    class array_access_node;
    class array_assignment_node;
    class variable_node;

    // flow control
    class return_node;
    class if_else_node;

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
    // text
    class char_node;
    class string_node;
    // other
    class bool_node;

    // operators
    // arithmetic
    class operator_addition_node;
    class operator_subtraction_node;
    class operator_multiplication_node;
    class operator_division_node;
    class operator_modulo_node;
    // logical
    class operator_conjunction_node;
    class operator_disjunction_node;
    class operator_greater_than_node;
    class operator_greater_than_equal_to_node;
    class operator_less_than_node;
    class operator_less_than_equal_to_node;
    class operator_equals_node;
    class operator_not_equals_node;
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
        virtual bool visit_access_node(access_node& node, value*& out_value) = 0;
        virtual bool visit_local_declaration_node(local_declaration_node& node, value*& out_value) = 0;
        virtual bool visit_global_declaration_node(global_declaration_node& node, value*& out_value) = 0;
        virtual bool visit_allocation_node(array_allocation_node& node, value*& out_value) = 0;
        virtual bool visit_array_access_node(array_access_node& node, value*& out_value) = 0;
        virtual bool visit_array_assignment_node(array_assignment_node& node, value*& out_value) = 0;
        virtual bool visit_variable_node(variable_node& node, value*& out_value) = 0;

        // flow control
        virtual bool visit_return_node(return_node& node, value*& out_value) = 0;
        virtual bool visit_if_else_node(if_else_node& node, value*& out_value) = 0;

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
        // text
        virtual bool visit_keyword_char_node(char_node& node, value*& out_value) = 0;
        virtual bool visit_keyword_string_node(string_node& node, value*& out_value) = 0;
        // other
    	virtual bool visit_keyword_bool_node(bool_node& node, value*& out_value) = 0;

        // operators
        // arithmetic
        virtual bool visit_operator_addition_node(operator_addition_node& node, value*& out_value) = 0;
        virtual bool visit_operator_subtraction_node(operator_subtraction_node& node, value*& out_value) = 0;
        virtual bool visit_operator_multiplication_node(operator_multiplication_node& node, value*& out_value) = 0;
        virtual bool visit_operator_division_node(operator_division_node& node, value*& out_value) = 0;
        virtual bool visit_operator_modulo_node(operator_modulo_node& node, value*& out_value) = 0;
        // logical
        virtual bool visit_operator_logical_conjunction_node(operator_conjunction_node& node, value*& out_value) = 0;
        virtual bool visit_operator_logical_disjunction_node(operator_disjunction_node& node, value*& out_value) = 0;
        virtual bool visit_operator_greater_than_node(operator_greater_than_node& node, value*& out_value) = 0;
        virtual bool visit_operator_greater_than_equal_to(operator_greater_than_equal_to_node& node, value*& out_value) = 0;
        virtual bool visit_operator_less_than_node(operator_less_than_node& node, value*& out_value) = 0;
        virtual bool visit_operator_less_than_equal_to_node(operator_less_than_equal_to_node& node, value*& out_value) = 0;
        virtual bool visit_operator_equals_node(operator_equals_node& node, value*& out_value) = 0;
        virtual bool visit_operator_not_equals_node(operator_not_equals_node& node, value*& out_value) = 0;
    };
}