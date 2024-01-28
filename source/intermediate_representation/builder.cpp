#include "builder.h"
#include <utility/string_helper.h>

// #define DEBUG_PRINT_BUILDER

#ifdef DEBUG_PRINT_BUILDER
#define DEBUG_PRINT(__fmt, ...) utility::console::print("{}\n", std::format(__fmt, ##__VA_ARGS__))
#else
#define DEBUG_PRINT(__fmt, ...)
#endif

namespace sigma::ir {
	builder::builder(module& target) : m_target(target) {}

	auto builder::create_function(const function_signature& signature, linkage linkage) -> handle<function> {
		DEBUG_PRINT("creating function");
		return m_insert_point = m_target.create_function(signature, linkage);
	}

	auto builder::create_call(handle<external> target, const function_signature& signature, const std::vector<handle<node>>& parameters) const -> handle<node> {
		DEBUG_PRINT("creating call (external)");
		return get_insert_point_checked()->create_call(target, signature, parameters);
	}

	auto builder::create_call(handle<function> target, const std::vector<handle<node>>& parameters) const -> handle<node> {
		DEBUG_PRINT("creating call (local)");
		return get_insert_point_checked()->create_call(target, parameters);
	}

	void builder::create_branch(handle<node> target) const {
		DEBUG_PRINT("creating branch");
		get_insert_point_checked()->create_branch(target);
	}

	void builder::create_conditional_branch(handle<node> condition, handle<node> if_true, handle<node> if_false) const {
		DEBUG_PRINT("creating conditional branch");
		return get_insert_point_checked()->create_conditional_branch(condition, if_true, if_false);
	}

	auto builder::create_region() const -> handle<node> {
		DEBUG_PRINT("creating region");
		return get_insert_point_checked()->create_region();
	}

	void builder::create_return(const std::vector<handle<node>>& return_values) const {
		ASSERT(return_values.size() <= 1, "invalid return count");
		DEBUG_PRINT("creating return");
		return get_insert_point_checked()->create_return(return_values);
	}

	auto builder::get_function_parameter(u64 index) const -> handle<node> {
		DEBUG_PRINT("creating function parameter");
		return get_insert_point_checked()->get_function_parameter(index);
	}

	auto builder::create_external(const std::string& identifier, linkage linkage) const -> handle<external> {
		DEBUG_PRINT("creating external");
		return m_target.create_external(identifier, linkage);
	}

	auto builder::create_external(const function_signature& signature,  linkage linkage) const -> handle<external> {
		DEBUG_PRINT("creating external");
		return m_target.create_external(signature.identifier, linkage);
	}

	auto builder::create_global(const std::string& identifier, linkage linkage) const -> handle<global> {
		DEBUG_PRINT("creating global");
		return m_target.create_global(identifier, linkage);
	}

	auto builder::create_local(u16 size, u16 alignment) const -> handle<node> {
		DEBUG_PRINT("creating local size: {} alignment: {}", size, alignment);
		return get_insert_point_checked()->create_local(size, alignment);
	}

	auto builder::create_string(const std::string& value) const -> handle<node> {
		DEBUG_PRINT("creating string '{}'", utility::detail::escape_string(value));
		return m_target.create_string(get_insert_point_checked(), value);
	}

	auto builder::create_signed_integer(i64 value, u8 bit_width) const -> handle<node> {
		DEBUG_PRINT("creating signed integer i{} {}", bit_width, value);
		return get_insert_point_checked()->create_signed_integer(value, bit_width);
	}

	auto builder::create_unsigned_integer(u64 value, u8 bit_width) const -> handle<node> {
		DEBUG_PRINT("creating unsigned integer u{} {}", bit_width, value);
		return get_insert_point_checked()->create_unsigned_integer(value, bit_width);
	}

	auto builder::create_bool(bool value) const -> handle<node> {
		DEBUG_PRINT("creating bool");
		return get_insert_point_checked()->create_bool(value);
	}

	auto builder::create_add(handle<node> left, handle<node> right, arithmetic_behaviour behaviour) const -> handle<node> {
		DEBUG_PRINT("creating add");
		return get_insert_point_checked()->create_add(left, right, behaviour);
	}

	auto builder::create_sub(handle<node> left, handle<node> right, arithmetic_behaviour behaviour) const -> handle<node> {
		DEBUG_PRINT("creating sub");
		return get_insert_point_checked()->create_sub(left, right, behaviour);
	}

	auto builder::create_mul(handle<node> left, handle<node> right, arithmetic_behaviour behaviour) const -> handle<node> {
		DEBUG_PRINT("creating mul");
		return get_insert_point_checked()->create_mul(left, right, behaviour);
	}

  auto builder::create_sxt(handle<node> src, data_type dt) const -> handle<node> {
		DEBUG_PRINT("creating sxt");
		return get_insert_point_checked()->create_sxt(src, dt);
  }

	auto builder::create_zxt(handle<node> src, data_type dt) const -> handle<node> {
		DEBUG_PRINT("creating zxt");
		return get_insert_point_checked()->create_zxt(src, dt);
	}

	auto builder::create_truncate(handle<node> src, data_type dt) const -> handle<node> {
		DEBUG_PRINT("creating truncate");
		return get_insert_point_checked()->create_truncate(src, dt);
	}

	void builder::create_store(handle<node> destination, handle<node> value, u32 alignment, bool is_volatile) const {
		DEBUG_PRINT("creating store alignment: {}", alignment);
		get_insert_point_checked()->create_store(destination, value, alignment, is_volatile);
	}

  auto builder::create_array_access(handle<node> base, handle<node> index, i64 stride) const  -> handle<node> { 
		DEBUG_PRINT("creating array access stride: {}", stride);
		return get_insert_point_checked()->create_array_access(base, index, stride);
  }

	auto builder::create_load(handle<node> value_to_load, data_type data_type, u32 alignment, bool is_volatile) const -> handle<node> {
		DEBUG_PRINT("creating load type: {} alignment: {}", data_type.to_string(), alignment);
		return get_insert_point_checked()->create_load(value_to_load, data_type, alignment, is_volatile);
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
