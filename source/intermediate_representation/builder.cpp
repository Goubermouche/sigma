#include "builder.h"

namespace ir {
	builder::builder(module& target) : m_target(target) {}

	auto builder::create_function(const function_type& type, linkage linkage) -> handle<function> {
		return m_insert_point = m_target.create_function(type, linkage);
	}

	auto builder::create_call(handle<external> target, const function_type& type, const std::vector<handle<node>>& parameters) const -> std::vector<handle<node>> {
		return get_insert_point_checked()->create_call(target, type, parameters);
	}

	auto builder::create_call(handle<function> target, const std::vector<handle<node>>& parameters) const -> std::vector<handle<node>> {
		return get_insert_point_checked()->create_call(target, parameters);
	}

	void builder::create_return(const std::vector<handle<node>>& return_values) const {
		return get_insert_point_checked()->create_ret(return_values);
	}

	auto builder::get_parameter(u64 index) const -> handle<node> {
		return get_insert_point_checked()->get_parameter(index);
	}

	auto builder::create_external(const std::string& identifier, linkage linkage) const -> handle<external> {
		return m_target.create_external(identifier, linkage);
	}

	auto builder::create_external(const function_type& type, linkage linkage) const -> handle<external> {
		return m_target.create_external(type.identifier, linkage);
	}

	auto builder::create_global(const std::string& identifier, linkage linkage) const -> handle<global> {
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

	auto builder::create_add(handle<node> left, handle<node> right, arithmetic_behaviour behaviour) const -> handle<node> {
		return get_insert_point_checked()->create_add(left, right, behaviour);
	}

	auto builder::get_insert_point() const -> handle<function> {
		return m_insert_point;
	}

	void builder::set_insert_point(handle<function> function) {
		ASSERT(function != nullptr, "specified insert point is null");
		m_insert_point = function;
	}

	auto builder::get_insert_point_checked() const -> handle<function> {
		ASSERT(m_insert_point, "unspecified insert point");
		return m_insert_point;
	}
}
