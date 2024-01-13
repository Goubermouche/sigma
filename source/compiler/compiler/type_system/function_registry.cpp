#include "function_registry.h"

#include <compiler/compiler/compilation_context.h>
#include <compiler/compiler/diagnostics.h>

#define INVALID_CAST_COST 1000

namespace sigma {
	function_registry::function_registry(backend_context& context) : m_context(context) {}

	bool function_registry::contains_function(const function_signature& signature) const {
		// try to find a locally declared function
		const auto local_it = m_local_functions.find(signature.identifier_key);
		if(local_it != m_local_functions.end()) {
			return local_it->second.contains(signature);
		}

		// fall back to external functions
		const auto external_it = m_external_functions.find(signature.identifier_key);
		if (external_it != m_external_functions.end()) {
			return external_it->second.contains(signature);
		}

		return false;
	}

	auto function_registry::get_callee_signature(const ast_function& function, const std::vector<data_type>& parameter_types) -> utility::result<function_signature> {
		using call_candidate = std::pair<function_signature, u16>;

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
						u16 cost = calculate_parameter_cast_cost(signature, parameter_types);

						if (cost < INVALID_CAST_COST) {
							candidates.emplace_back(signature, cost);
						}
					}
				}
			}
		};

		add_candidates(m_local_functions);
		add_candidates(m_external_functions);

		if(!valid_identifier) {
			// no function with the specified identifier was found
			const std::string& identifier_str = m_context.strings.get(function.signature.identifier_key);
			return error::emit(error::code::UNKNOWN_FUNCTION, identifier_str);
		}

		if(candidates.empty()) {
			return emit_no_viable_overload_error(function);
		}

		const auto best_match = std::min_element(
			candidates.begin(), candidates.end(),
			[](const call_candidate& a, const call_candidate& b) { return a.second < b.second; }
		);

		// TODO: implement casting
		ASSERT(best_match->second == 0, "implement casting in the type checker!");
		return best_match->first;
	}

	void function_registry::declare_external_function(const function_signature& signature) {
		const std::string& identifier = m_context.strings.get(signature.identifier_key);

		m_external_functions[signature.identifier_key][signature] = {
			.ir_function = m_context.module.create_external(identifier, ir::linkage::SO_LOCAL),
			.ir_signature = signature_to_ir(signature, m_context.strings)
		};
	}

	void function_registry::declare_local_function(const function_signature& signature) {
		const ir::function_signature ir_signature = signature_to_ir(signature, m_context.strings);
		const	handle<ir::function> function = m_context.builder.create_function(ir_signature, ir::linkage::PUBLIC);

		m_local_functions.at(signature.identifier_key).at(signature) = function;
	}

	void function_registry::pre_declare_local_function(const function_signature& signature) {
		m_local_functions[signature.identifier_key][signature] = nullptr;
	}

	auto function_registry::create_call(const function_signature& callee_signature, const std::vector<handle<ir::node>>& parameters) -> handle<ir::node> {
		// attempt to call a local function
		const auto local_it = m_local_functions.find(callee_signature.identifier_key);
		if (local_it != m_local_functions.end()) {
			return m_context.builder.create_call(local_it->second.at(callee_signature), parameters);
		}

		// attempt to call an external function
		const auto external_it = m_external_functions.find(callee_signature.identifier_key);
		if (external_it != m_external_functions.end()) {
			const external_function& external = external_it->second.at(callee_signature);
			return m_context.builder.create_call(external.ir_function, external.ir_signature, parameters);
		}

		PANIC("unknown function called");
		return nullptr;
	}

	auto function_registry::emit_no_viable_overload_error(const ast_function& function) -> utility::error {
		// construct a list of all potentially viable functions
		std::stringstream candidate_stream;
		const std::string& identifier_str = m_context.strings.get(function.signature.identifier_key);

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
							<< m_context.strings.get(signature.parameter_types[i].identifier_key);

						if (i + 1 != signature.parameter_types.get_size()) {
							candidate_stream << ", ";
						}
					}

					candidate_stream << ")\n";
				}
			}
		};

		add_considered_candidate(m_local_functions);
		add_considered_candidate(m_external_functions);

		// TODO: when we get allocator-based strings working, remove the trailing \n
		return error::emit(error::code::NO_CALL_OVERLOAD, function.location, identifier_str, candidate_stream.str());
	}

	auto function_registry::calculate_parameter_cast_cost(const function_signature& signature, const std::vector<data_type>& parameter_types) -> u16 {
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

	auto function_registry::calculate_cast_cost(const data_type& provided, const data_type& required) -> u16 {
		// if types are the same, no cast is needed
		if (provided.base_type == required.base_type) {
			return 0;
		}

		if (provided.pointer_level != required.pointer_level) {
			return INVALID_CAST_COST; // invalid operation
		}

		const u16 provided_width = provided.get_byte_width();
		const u16 required_width = required.get_byte_width();

		// handle casting between integer types
		if (provided.is_integer() && required.is_integer()) {
			if (provided_width < required_width) {
				// widening cast, lower cost
				return 1 * (required_width - provided_width);
			}

			if (provided_width > required_width) {
				// narrowing cast, higher cost
				return 2 * (provided_width - required_width);
			}
		}

		return INVALID_CAST_COST; // invalid operation
	}

	auto data_type_to_ir(data_type type) -> ir::data_type {
		if(type.pointer_level == 1) {
			return PTR_TYPE;
		}

		ASSERT(type.pointer_level <= 1, "invalid pointer level");

		switch (type.base_type) {
			case data_type::I8:   return I8_TYPE;
			case data_type::I16:  return I16_TYPE;
			case data_type::I32:  return I32_TYPE;
			case data_type::I64:  return I64_TYPE;
			case data_type::BOOL: return BOOL_TYPE;
			default: NOT_IMPLEMENTED();
		}

		return {};
	}

	auto signature_to_ir(const function_signature& signature, const utility::string_table& string_table) -> ir::function_signature {
		std::vector<ir::data_type> parameters(signature.parameter_types.get_size());

		for(u64 i = 0; i < signature.parameter_types.get_size(); ++i) {
			// at this point we can get rid of the name
			parameters[i] = data_type_to_ir(signature.parameter_types[i].type);
		}

		ir::function_signature ir_signature {
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
		if(string_table.get(signature.identifier_key) == "main") {
			return "main";
		}

		static u64 counter = 0;
		return "f" + std::to_string(counter++);
	}
} // namespace sigma
