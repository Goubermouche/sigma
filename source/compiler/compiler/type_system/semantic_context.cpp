#include "semantic_context.h"

#include <compiler/compiler/compilation_context.h>
#include <compiler/compiler/diagnostics.h>

#define INVALID_CAST_COST 1000

namespace sigma {
	namespace detail {
		auto data_type_to_ir(type type) -> ir::data_type {
			if (type.is_pointer()) {
				return PTR_TYPE;
			}

			switch (type.get_kind()) {
				case type::VOID:   return VOID_TYPE;
				case type::BOOL:
				case type::I8:
				case type::U8:     return I8_TYPE;
				case type::I16:
				case type::U16:    return I16_TYPE;
				case type::CHAR:
				case type::I32:
				case type::U32:    return I32_TYPE;
				case type::I64:
				case type::U64:    return I64_TYPE;
				case type::STRUCT: return PTR_TYPE;
				default: NOT_IMPLEMENTED();
			}

			return {};
		}

		auto signature_to_ir(const function_signature& signature, const utility::string_table& string_table) -> ir::function_signature {
			std::vector<ir::data_type> parameters(signature.parameter_types.get_size());

			for (u64 i = 0; i < signature.parameter_types.get_size(); ++i) {
				// at this point we can get rid of the name
				parameters[i] = data_type_to_ir(signature.parameter_types[i].type);
			}

			ir::function_signature ir_signature{
				.identifier = mangle_function_identifier(signature, string_table),
				.parameters = parameters,
				.returns = { data_type_to_ir(signature.return_type) },
				.has_var_args = signature.has_var_args
			};

			return ir_signature;
		}

		auto mangle_function_identifier(const function_signature& signature, const utility::string_table& string_table) -> std::string {
			// TODO: names are a bit sus right now, longer names don't appear ot be stored correctly in
			//       object files (see COFF & ELF)

			// don't rename the main function
			if (string_table.get(signature.identifier_key) == "main") {
				return "main";
			}

			static u64 counter = 0;
			return "f" + std::to_string(counter++);
			//return string_table.get(signature.identifier_key);
		}

		auto calculate_parameter_cast_cost(const function_signature& signature, const std::vector<type>& parameter_types) -> u16 {
			// NOTE: right now, we just traverse all non-var arg type and match compare against those, it's
			//       probably a good idea to add a heavy-ish cost (~200) to var-arg functions, as we want to
			//       prefer non var-arg functions by default (?)

			u16 total_cost = 0;

			for (u64 i = 0; i < signature.parameter_types.get_size(); ++i) {
				// calculate the cost for this parameter
				total_cost += calculate_cast_cost(parameter_types[i], signature.parameter_types[i].type);
			}

			return total_cost;
		}

		auto calculate_cast_cost(const type& provided, const type& required) -> u16 {
			// if types are the same, no cast is needed
			if (provided == required) {
				return 0;
			}

			if (provided.get_pointer_level() != required.get_pointer_level()) {
				return INVALID_CAST_COST; // invalid operation
			}

			const u16 provided_width = provided.get_size();
			const u16 required_width = required.get_size();

			// handle casting between integer types
			if (provided.is_integral() && required.is_integral()) {
				if (provided_width < required_width) {
					// widening cast, lower cost
					return 1 * (required_width - provided_width);
				}

				if (provided_width > required_width) {
					// narrowing cast, higher cost
					return 2 * (provided_width - required_width);
				}

				// the types aren't equal, but their bit width is - sign difference
				return 15;
			}

			return INVALID_CAST_COST; // invalid operation
		}

		auto determine_cast_kind(const type& original, const type& target) -> bool {
			const u16 original_byte_width = original.get_size();
			const u16 target_byte_width   = target.get_size();

			return original_byte_width > target_byte_width;
		}
	} // namespace detail

  semantic_context::semantic_context(backend_context& context) : m_context(context) {
		m_global_scope = allocate_namespace();
		reset_active_scope();
  }

	auto semantic_context::verify_control_flow(handle<ast::node> function_node) const -> utility::result<void> {
		const ast::function function = function_node->get<ast::function>();

		// check if all control paths return
		if(!function.signature.return_type.is_pure_void()) {
			// edge case for top level scopes
			if(!m_current_scope->has_return && m_current_scope->child_scopes.empty()) {
				return error::emit(error::code::NOT_ALL_CONTROL_PATHS_RETURN, function_node->location);
			}

			TRY(all_control_paths_return(m_current_scope, function_node));
		}

		return SUCCESS;
	}

  void semantic_context::push_scope(scope::control_type control) {
		const handle new_scope = allocate_scope();
		new_scope->parent = m_current_scope;
		new_scope->control = control;

		m_current_scope->child_scopes.push_back(new_scope);
		m_current_scope = new_scope;
		m_trace.emplace_back(new_scope);
  }

	void semantic_context::push_namespace(utility::string_table_key name) {
		const handle new_scope = allocate_namespace();
		new_scope->parent = m_current_scope;

		ASSERT(m_current_scope->scope_ty == scope::scope_type::NAMESPACE, "cannot declare a namespace in a non-namespace scope");

		const handle<namespace_scope> current = m_current_scope;
		current->child_namespaces[name] = new_scope;

		m_current_scope = new_scope;
		m_trace.emplace_back(new_scope);
	}

	void semantic_context::pop_scope() {
		ASSERT(m_current_scope->parent, "invalid pop on global scope");
		m_current_scope = m_current_scope->parent;
	}

	void semantic_context::trace_push_scope() {
		m_current_scope = m_trace[m_trace_index++];
	}

	void semantic_context::trace_pop_scope() {
		pop_scope();
	}

	void semantic_context::reset_active_scope() {
		m_current_scope = m_global_scope;
		m_trace_index = 0;
	}

	auto semantic_context::pre_declare_variable(utility::string_table_key identifier, type type) const -> variable& {
		auto& var = m_current_scope->variables[identifier];
		var.type = type;
		return var;
	}

	auto semantic_context::declare_variable(utility::string_table_key identifier, u16 size, u16 alignment) const -> handle<ir::node> {
		return m_current_scope->variables.at(identifier).value = m_context.builder.create_local(size, alignment);
	}

	auto semantic_context::find_variable(utility::string_table_key identifier, const namespace_list& namespaces) const -> utility::result<handle<variable>> {
  	const handle<scope> root_scope = find_namespace(namespaces);

		if (!root_scope) {
			return emit_unknown_namespace_error(namespaces);
		}

		// NOTE: may be null, this is handled after this function is called depending on the context of
		// where this function was called (ie. we want to emit different errors depending on whether
		// we're accessing a variable or assigning to it)
		return root_scope->find_variable(identifier);
	}

	auto semantic_context::get_variable(utility::string_table_key identifier, const namespace_list& namespaces) const -> handle<variable> {
		// TODO: this is a temporary function, we should probably replace this by just referencing
		//       the variable in the type checker right away and using that
		return find_namespace(namespaces)->find_variable(identifier);
	}

	auto semantic_context::create_load(utility::string_table_key identifier, ir::data_type type, u16 alignment) const -> handle<ir::node> {
		const handle<variable> variable = m_current_scope->find_variable(identifier);
		ASSERT(variable, "attempting to load an invalid variable");
		return m_context.builder.create_load(variable->value, type, alignment, false);
  }

	void semantic_context::create_store(utility::string_table_key identifier, handle<ir::node> value, u16 alignment) const {
		const handle<variable> variable = m_current_scope->find_variable(identifier);
		// ASSERT(!(variable->flags & variable::FUNCTION_PARAMETER), "not implemented");

		if (variable != nullptr) {
			m_context.builder.create_store(variable->value, value, alignment, false);
			return;
		}

		ASSERT(false, "unknown variable referenced");
  }

	auto semantic_context::resolve_type(type& ty, handle<token_location> location) const -> utility::result<void> {
		if(!ty.is_unresolved()) {
			return SUCCESS; // nothing else needed
		}

		if(const handle<type> resolved = m_current_scope->find_type(ty.get_unresolved())) {
			ty.set_kind(resolved->get_kind());

			if(resolved->is_struct()) {
				ty.set_struct_members(resolved->get_struct_members());
			}

			return SUCCESS;
		}

		const std::string& identifier = m_context.syntax.strings.get(ty.get_unresolved());
		return error::emit(error::code::UNKNOWN_TYPE, location, identifier);
	}

	auto semantic_context::find_namespace(const namespace_list& namespaces) const->handle<scope> {
		if (!namespaces.empty()) {
			const handle<namespace_scope> current_namespace = find_parent_namespace();

			const auto it = current_namespace->child_namespaces.find(namespaces.first());
			if (it != current_namespace->child_namespaces.end()) {
				return it->second->find_namespace(namespaces, 1);
			}

			return nullptr;
		}

		return m_current_scope;
  }

	auto semantic_context::find_parent_namespace() const -> handle<namespace_scope> {
		if(m_current_scope->scope_ty == scope::scope_type::NAMESPACE) {
			return m_current_scope;
		}

		return m_current_scope->find_parent_namespace();
	}

	auto semantic_context::find_relative_namespace(const namespace_list& namespaces) const -> handle<namespace_scope> {
		const handle<namespace_scope> current_namespace = find_parent_namespace();

		if(namespaces.empty()) {
			return current_namespace;
		}

		const auto it = current_namespace->child_namespaces.find(namespaces.first());
		if (it != current_namespace->child_namespaces.end()) {
			return it->second->find_namespace(namespaces, 1);
		}

		// unknown namespace
		return nullptr;
	}

	auto semantic_context::allocate_scope() const -> handle<scope> {
		return m_context.allocator.emplace<scope>(scope::scope_type::REGULAR);
	}

	auto semantic_context::allocate_namespace() const -> handle<scope> {
		return m_context.allocator.emplace<namespace_scope>(scope::scope_type::NAMESPACE);
	}

	void semantic_context::pre_declare_local_function(const function_signature& signature) const {
		find_parent_namespace()->local_functions[signature.identifier_key][signature] = nullptr;
	}

	bool semantic_context::contains_variable(utility::string_table_key identifier) const {
		return m_current_scope->find_variable(identifier) != nullptr;
	}

  auto semantic_context::declare_struct(handle<ast::node> node) const -> utility::result<void> {
		const auto& expression = node->get<ast::named_type_expression>();

		// check if the struct hasn't already been defined in this scope
		if(m_current_scope->find_type(expression.key) != nullptr) {
			const std::string& identifier = m_context.syntax.strings.get(expression.key);
			return error::emit(error::code::STRUCT_ALREADY_DECLARED, node->location, identifier);
		}

		m_current_scope->types[expression.key] = expression.type;
		return SUCCESS;
  }

	bool semantic_context::contains_function(const function_signature& signature) const {
		const handle<namespace_scope> scope = find_parent_namespace();

		// try to find a locally declared function
		const auto local_it = scope->local_functions.find(signature.identifier_key);
		if (local_it != scope->local_functions.end()) {
			return local_it->second.contains(signature);
		}

		// fall back to external functions
		const auto external_it = scope->external_functions.find(signature.identifier_key);
		if (external_it != scope->external_functions.end()) {
			return external_it->second.contains(signature);
		}

		return false;
	}

	bool semantic_context::has_return() const {
		return m_current_scope->has_return;
	}

	void semantic_context::declare_return() const {
		m_current_scope->has_return = true;
	}

	auto semantic_context::all_control_paths_return(handle<scope> scope, handle<ast::node> function_node) -> utility::result<bool> {
		if(scope->has_return) {
			// all control paths in this scope return a value by default
			return true;
		}

		u64 return_count = 0;
		bool has_unconditional_return = false;

		// check how many child scopes return a value
		for(const auto& child : scope->child_scopes) {
			if(child->has_return) {
				return_count++;
			}
			else {
				TRY(const bool has_return, all_control_paths_return(child, function_node));
				return_count += has_return;
			}

			has_unconditional_return |= child->control == scope::control_type::UNCONDITIONAL;
		}

		// if the return count of child scopes and the expected number of scopes
		// don't match, we've got an error
		if(return_count != scope->child_scopes.size()) {
			return error::emit(error::code::NOT_ALL_CONTROL_PATHS_RETURN, function_node->location);
		}

		// if the number of returns matches, but we don't have an unconditional return
		// we've got an error as well
		if(!has_unconditional_return) {
			return error::emit(error::code::NOT_ALL_CONTROL_PATHS_RETURN, function_node->location);
		}

		// all control paths in this scope can return if it even has any control paths
		return !scope->child_scopes.empty();
	}

	auto semantic_context::find_callee_signature(handle<ast::node> function_node, const std::vector<type>& parameter_types) -> utility::result<function_signature> {
		using call_candidate = std::pair<function_signature, u16>;

		const ast::function_call& function = function_node->get<ast::function_call>();
		std::vector<call_candidate> candidates;
		bool valid_identifier = false;

		// helper for getting potential candidates
		auto add_candidates = [&](const auto& function_map) {
			const auto it = function_map.find(function.signature.identifier_key);
			if (it != function_map.end()) {
				valid_identifier = true;

				for (const auto& [signature, ir_function] : it->second) {
					if (
						signature.parameter_types.get_size() == parameter_types.size() ||
						(signature.parameter_types.get_size() < parameter_types.size() && signature.has_var_args)
					) {
						u16 cost = detail::calculate_parameter_cast_cost(signature, parameter_types);

						if(cost < INVALID_CAST_COST) {
							candidates.emplace_back(signature, cost);
						}
					}
				}
			}
		};

		const handle<namespace_scope> scope = find_relative_namespace(function.namespaces);

		if(scope == nullptr) {
			// invalid namespace
			return emit_unknown_namespace_error(function.namespaces);
		}

		add_candidates(scope->local_functions);
		add_candidates(scope->external_functions);

		if (!valid_identifier) {
			// no function with the specified identifier was found
			std::stringstream function_name = function.namespaces.get_stream(m_context.syntax.strings);
			function_name << "::";
			function_name << m_context.syntax.strings.get(function.signature.identifier_key);

			return error::emit(error::code::UNKNOWN_FUNCTION, function_name.str());
		}

		if (candidates.empty()) {
			return emit_no_viable_overload_error(function_node);
		}

		const auto best_match = std::min_element(
			candidates.begin(), candidates.end(),
			[](const call_candidate& a, const call_candidate& b) { return a.second < b.second; }
		);

		// TODO: implement casting
		// TODO: literals can just be upcasted implicitly
		// ASSERT(best_match->second == 0, "implement casting in the type checker!");
		return best_match->first;
	}

	void semantic_context::declare_external_function(const function_signature& signature) const {
		const std::string& identifier = m_context.syntax.strings.get(signature.identifier_key);

		find_parent_namespace()->external_functions[signature.identifier_key][signature] = {
			.ir_function = m_context.module.create_external(identifier, ir::linkage::SO_LOCAL),
			.ir_signature = detail::signature_to_ir(signature, m_context.syntax.strings)
		};
	}

	void semantic_context::declare_local_function(const function_signature& signature) const {
		const ir::function_signature ir_signature = detail::signature_to_ir(signature, m_context.syntax.strings);
		const	handle<ir::function> function = m_context.builder.create_function(ir_signature, ir::linkage::PUBLIC);

		find_parent_namespace()->local_functions.at(signature.identifier_key).at(signature) = function;
	}

	void semantic_context::define_implicit_return() const {
		// declare an implicit return value for the active function
		const handle<ir::function> function = m_context.builder.get_insert_point();

		if(function->exit_node == nullptr) {
			// declare an implicit return value
			m_context.builder.create_return({});
		}
	}

	auto semantic_context::create_call(const function_signature& callee_signature, const namespace_list& namespaces, const std::vector<handle<ir::node>>& parameters) const -> handle<ir::node> {
		const handle<namespace_scope> scope = find_relative_namespace(namespaces);
		ASSERT(scope, "invalid namespace");

		// attempt to call a local function
		const auto local_it = scope->local_functions.find(callee_signature.identifier_key);
		if (local_it != scope->local_functions.end()) {
			return m_context.builder.create_call(local_it->second.at(callee_signature), parameters);
		}

		// attempt to call an external function
		const auto external_it = scope->external_functions.find(callee_signature.identifier_key);
		if (external_it != scope->external_functions.end()) {
			const external_function& external = external_it->second.at(callee_signature);
			return m_context.builder.create_call(external.ir_function, external.ir_signature, parameters);
		}

		PANIC("unknown function called");
		return nullptr;
	}

	auto semantic_context::emit_no_viable_overload_error(handle<ast::node> function_node) -> utility::error {
		const ast::function_call& function = function_node->get<ast::function_call>();

		// construct a list of all potentially viable functions
		std::stringstream candidate_stream;
		const std::string& identifier_str = m_context.syntax.strings.get(function.signature.identifier_key);

		auto add_considered_candidate = [&](const auto& function_map) {
			const auto it = function_map.find(function.signature.identifier_key);

			if (it != function_map.end()) {
				for (const auto& [signature, ir_function] : it->second) {
					candidate_stream
						<< "  "
						<< signature.return_type.to_string()
						<< " "
						<< identifier_str
						<< "(";

					for (u64 i = 0; i < signature.parameter_types.get_size(); ++i) {
						candidate_stream
							<< signature.parameter_types[i].type.to_string()
							<< " "
							<< m_context.syntax.strings.get(signature.parameter_types[i].identifier_key);

						if (i + 1 != signature.parameter_types.get_size()) {
							candidate_stream << ", ";
						}
					}

					if(signature.has_var_args) {
						candidate_stream << ", ...";
					}

					candidate_stream << ")\n";
				}
			}
		};

		const handle<namespace_scope> scope = find_parent_namespace();

		add_considered_candidate(scope->local_functions);
		add_considered_candidate(scope->external_functions);

		// TODO: when we get allocator-based strings working, remove the trailing \n
		return error::emit(error::code::NO_CALL_OVERLOAD, function_node->location, identifier_str, candidate_stream.str());
	}

	auto semantic_context::emit_unknown_namespace_error(const namespace_list& namespaces) const -> utility::error {
		return error::emit(error::code::UNKNOWN_NAMESPACE, namespaces.get_stream(m_context.syntax.strings).str());
	}
} // namespace sigma
