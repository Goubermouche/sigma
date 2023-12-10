#include "builder.h"

namespace sigma::ir {
	builder::builder(module& target) : m_target(target) {}

	auto builder::create_function(
		const function_signature& function_sig, 
		linkage linkage
	) -> handle<function> {
		return m_insert_point = m_target.create_function(function_sig, linkage);
	}

	auto builder::create_call(
		handle<external> target,
		const function_signature& function_sig, 
		const std::vector<handle<node>>& parameters
	) const -> handle<node> {
		return get_insert_point_checked()->create_call(target, function_sig, parameters);
	}

	auto builder::create_call(
		handle<function> target, 
		const std::vector<handle<node>>& parameters
	) const -> handle<node> {
		return get_insert_point_checked()->create_call(target, parameters);
	}

	void builder::create_branch(handle<node> target) const {
		get_insert_point_checked()->create_branch(target);
	}

	void builder::create_conditional_branch(
		handle<node> condition,
		handle<node> if_true,
		handle<node> if_false
	) const {
		return get_insert_point_checked()->create_conditional_branch(condition, if_true, if_false);
	}

	auto builder::create_region() const -> handle<node> {
		return get_insert_point_checked()->create_region();
	}

	void builder::create_return(const std::vector<handle<node>>& return_values) const {
		return get_insert_point_checked()->create_ret(return_values);
	}

	auto builder::get_function_parameter(u64 index) const -> handle<node> {
		return get_insert_point_checked()->get_function_parameter(index);
	}

	auto builder::create_external(
		const std::string& identifier,
		linkage linkage
	) const -> handle<external> {
		return m_target.create_external(identifier, linkage);
	}

	auto builder::create_external(
		const function_signature& function_sig, 
		linkage linkage
	) const -> handle<external> {
		return m_target.create_external(function_sig.identifier, linkage);
	}

	auto builder::create_global(
		const std::string& identifier, 
		linkage linkage
	) const -> handle<global> {
		return m_target.create_global(identifier, linkage);
	}

	auto builder::create_local(u16 size, u16 alignment) const -> handle<node> {
		return get_insert_point_checked()->create_local(size, alignment);
	}

	auto builder::create_string(const std::string& value) const -> handle<node> {
		return m_target.create_string(get_insert_point_checked(), value);
	}

	auto builder::create_signed_integer(i64 value, u8 bit_width) const -> handle<node> {
		return get_insert_point_checked()->create_signed_integer(value, bit_width);
	}

	auto builder::create_bool(bool value) const -> handle<node> {
		return get_insert_point_checked()->create_bool(value);
	}

	auto builder::create_add(
		handle<node> left,
		handle<node> right, 
		arithmetic_behaviour behaviour
	) const -> handle<node> {
		return get_insert_point_checked()->create_add(left, right, behaviour);
	}

	auto builder::create_sub(
		handle<node> left,
		handle<node> right,
		arithmetic_behaviour behaviour
	) const -> handle<node> {
		return get_insert_point_checked()->create_sub(left, right, behaviour);
	}

	auto builder::create_mul(
		handle<node> left,
		handle<node> right,
		arithmetic_behaviour behaviour
	) const -> handle<node> {
		return get_insert_point_checked()->create_mul(left, right, behaviour);
	}

	void builder::create_store(
		handle<node> destination, handle<node> value, u32 alignment, bool is_volatile
	) const {
		get_insert_point_checked()->create_store(destination, value, alignment, is_volatile);
	}

	auto builder::create_load(
		handle<node> value_to_load, data_type data_type, u32 alignment, bool is_volatile
	) const -> handle<node> {
		return get_insert_point_checked()->create_load(
			value_to_load, data_type, alignment, is_volatile
		);
	}

	auto builder::get_insert_point() const -> handle<function> {
		return m_insert_point;
	}

	void builder::set_insert_point(handle<function> function) {
		ASSERT(function != nullptr, "specified insert point is null");
		m_insert_point = function;
	}

	void builder::set_control(handle<node> control) const {
		get_insert_point_checked()->active_control_node = control;
	}

	auto builder::get_insert_point_checked() const -> handle<function> {
		ASSERT(m_insert_point, "unspecified insert point");
		return m_insert_point;
	}
} // namespace sigma::ir
