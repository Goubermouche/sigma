#include "ir_translator.h"
#include <compiler/compiler/compilation_context.h>

namespace sigma {
	auto ir_translator::translate(
		compilation_context& context, ir::target target
	) -> ir::module { 
		return ir_translator(context, target).translate();
	}

	ir_translator::ir_translator(
		compilation_context& context, ir::target target
	) : m_context(context), m_module(target), m_builder(m_module),
	m_functions(m_builder), m_variables(m_builder) {

		m_functions.register_external_function(m_context.symbols.insert("printf"), {
			.identifier   = "printf",
			.parameters   = { PTR_TYPE },
			.returns      = { I32_TYPE },
			.has_var_args = true
		});

		m_functions.register_external_function(m_context.symbols.insert("puts"), {
			.identifier   = "puts",
			.parameters   = { PTR_TYPE },
			.returns      = { I32_TYPE },
			.has_var_args = false
		});
	}

	handle<ir::node> ir_translator::translate_node(handle<node> ast_node) {
		switch (ast_node->type) {
			case node_type::FUNCTION:             translate_function_declaration(ast_node); break;
			case node_type::FUNCTION_CALL:        return translate_function_call(ast_node);

			// flow control
			case node_type::RETURN:               translate_return(ast_node); break;
			case node_type::CONDITIONAL_BRANCH:   translate_conditional_branch(ast_node, nullptr); break;

			case node_type::VARIABLE_DECLARATION: translate_variable_declaration(ast_node); break;
			case node_type::VARIABLE_ACCESS:      return translate_variable_access(ast_node);
			case node_type::VARIABLE_ASSIGNMENT:  return translate_variable_assignment(ast_node);

			// operators:
			case node_type::OPERATOR_ADD:
			case node_type::OPERATOR_SUBTRACT:
			case node_type::OPERATOR_MULTIPLY:
			case node_type::OPERATOR_DIVIDE:
			case node_type::OPERATOR_MODULO:      return translate_binary_math_operator(ast_node);

			// literals
			case node_type::NUMERICAL_LITERAL:    return translate_numerical_literal(ast_node);
			case node_type::STRING_LITERAL:       return translate_string_literal(ast_node);
			case node_type::BOOL_LITERAL:         return translate_bool_literal(ast_node);
			default: NOT_IMPLEMENTED();
		}

		return nullptr;
	}

	void ir_translator::translate_function_declaration(handle<node> function_node) {
		auto& prop = function_node->get<function>();

		std::vector<ir::data_type> parameter_types(prop.parameter_types.get_size());

		for (u64 i = 0; i < parameter_types.size(); ++i) {
			parameter_types[i] = data_type_to_ir(prop.parameter_types[i].type);
		}

		// TODO: the IR system should inherit our symbol table system
		const ir::function_signature signature {
			.identifier = m_context.symbols.get(prop.identifier_key), // TEMP
			.parameters = parameter_types,
			.returns = { data_type_to_ir(prop.return_type) },
			.has_var_args = false
		};

		m_functions.register_function(prop.identifier_key, signature);

		// handle inner statements
		for (const handle<node> statement : function_node->children) {
			translate_node(statement);
		}

		// TODO: safety checks?
	}

	void ir_translator::translate_variable_declaration(handle<node> variable_node) {
		const auto& prop = variable_node->get<variable>();
		const u16 byte_width = prop.data_type.get_byte_width();

		const handle<ir::node> local = m_variables.register_variable(
			prop.identifier_key, byte_width, byte_width
		);

		if (variable_node->children.get_size() == 1) {
			const handle<ir::node> expression = translate_node(variable_node->children[0]);
			m_builder.create_store(local, expression, byte_width, false);
		}
		else if (prop.data_type.type == data_type::BOOL) {
			// boolean values should default to false
			m_builder.create_store(local, m_builder.create_bool(false), byte_width, false);
		}
	}

	void ir_translator::translate_return(handle<node> return_node) {
		if (return_node->children.get_size() == 0) {
			m_builder.create_return({}); // TODO: maybe this should return a VOID_TY?
		}
		else {
			m_builder.create_return({ translate_node(return_node->children[0]) });
		}
	}

	void ir_translator::translate_conditional_branch(
		handle<node> branch_node, handle<ir::node> end_control
	) {
		const handle<ir::node> true_control = m_builder.create_region();
		const handle<ir::node> false_control = m_builder.create_region();
		end_control = end_control ? end_control : m_builder.create_region();

		// translate the condition
		const handle<ir::node> condition = translate_node(branch_node->children[0]);

		// check if there is a successor branch node
		if (const handle<node> successor = branch_node->children[1]) {
			// successor node
			m_builder.create_conditional_branch(condition, true_control, false_control);
			m_builder.set_control(false_control);

			[[likely]]
			if (successor->type == node_type::CONDITIONAL_BRANCH) {
				// else if
				translate_conditional_branch(successor, end_control);
			}
			else if (successor->type == node_type::BRANCH) {
				// else
				translate_branch(successor, end_control);
			}
			else {
				PANIC("unreachable");
			}
		}
		else {
			m_builder.create_conditional_branch(condition, true_control, end_control);
		}

		// this all happens if CONDITION IS true
		m_builder.set_control(true_control);

		for (u64 i = 2; i < branch_node->children.get_size(); ++i) {
			translate_node(branch_node->children[i]);
		}

		m_builder.create_branch(end_control);

		// restore the control region
		m_builder.set_control(end_control);
	}

	void ir_translator::translate_branch(handle<node> branch_node, handle<ir::node> exit_control) {
		for (const handle<node> statement : branch_node->children) {
			translate_node(statement);
		}

		m_builder.create_branch(exit_control);
	}

	auto ir_translator::translate_numerical_literal(
		handle<node> numerical_literal_node
	) const -> handle<ir::node> {
		return literal_to_ir(numerical_literal_node->get<literal>());
	}

	auto ir_translator::translate_string_literal(
		handle<node> string_literal_node
	) const -> handle<ir::node> {
		const std::string& value = m_context.symbols.get(string_literal_node->get<literal>().value_key);
		return m_builder.create_string(value);
	}

	auto ir_translator::translate_bool_literal(
		handle<node> bool_literal_node
	) const -> handle<ir::node> {
		const auto& prop = bool_literal_node->get<bool_literal>();
		return m_builder.create_bool(prop.value);
	}

	auto ir_translator::translate_binary_math_operator(
		handle<node> operator_node
	) -> handle<ir::node> {
		const handle<ir::node> left  = translate_node(operator_node->children[0]);
		const handle<ir::node> right = translate_node(operator_node->children[1]);

		switch (operator_node->type) {
			case node_type::OPERATOR_ADD:      return m_builder.create_add(left, right);
			case node_type::OPERATOR_SUBTRACT: return m_builder.create_sub(left, right);
			case node_type::OPERATOR_MULTIPLY: return m_builder.create_mul(left, right);
			//case node_type::OPERATOR_DIVIDE:   
			//case node_type::OPERATOR_MODULO:   
		}

		NOT_IMPLEMENTED();
		return nullptr;
	}

	auto ir_translator::translate_function_call(handle<node> call_node) -> handle<ir::node> {
		const auto& prop = call_node->get<function_call>();

		std::vector<handle<ir::node>> parameters;
		parameters.reserve(call_node->children.get_size());

		for (const handle<node> parameter : call_node->children) {
			parameters.push_back(translate_node(parameter));
		}

		handle<ir::node> call_result = m_functions.create_call(prop.callee_identifier_key, parameters);
		ASSERT(call_result, "unknown function called");
		return call_result;
	}

	auto ir_translator::translate_variable_access(handle<node> access_node) -> handle<ir::node> {
		const auto& prop = access_node->get<variable_access>();

		handle<ir::node> load = m_variables.create_load(
			prop.identifier_key, data_type_to_ir(prop.data_type), prop.data_type.get_byte_width()
		);

		ASSERT(load, "unknown variable referenced");
		return load;
	}

	auto ir_translator::translate_variable_assignment(
		handle<node> assignment_node
	) -> handle<ir::node> {
		const auto& var = assignment_node->children[0]->get<variable_access>();
		const handle<ir::node> value = translate_node(assignment_node->children[1]);

		// assign the variable
		m_variables.create_store(var.identifier_key, value, var.data_type.get_byte_width());
		return nullptr;
	}

	auto ir_translator::literal_to_ir(literal& literal) const -> handle<ir::node> {
		const std::string& value = m_context.symbols.get(literal.value_key);

		// handle pointers separately
		if (literal.data_type.pointer_level > 0) {
			NOT_IMPLEMENTED();
		}

		switch (literal.data_type.type) {
			case data_type::I32: return m_builder.create_signed_integer(std::stoi(value), 32);
			default: NOT_IMPLEMENTED();
		}

		return nullptr;
	}

	auto ir_translator::data_type_to_ir(data_type dt) -> ir::data_type {
		ASSERT(dt.pointer_level == 0, "invalid pointer level");

		switch (dt.type) {
			case data_type::I32:  return I32_TYPE;
			case data_type::BOOL: return BOOL_TYPE;
			default: NOT_IMPLEMENTED();
		}

		return {};
	}

	auto ir_translator::translate() -> ir::module {
		for (const handle<node> top_level : m_context.ast.get_nodes()) {
			translate_node(top_level);
		}

		return std::move(m_module);
	}
} // namespace sigma
