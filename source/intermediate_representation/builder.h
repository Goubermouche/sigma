#pragma once
#include "intermediate_representation/module.h"

// Simple wrapper utility for the creation of various nodes in a given module. Basic usage:
// 
//   ir::module = ...
//   ir::builder builder(module);
//
//   builder.create_function(...); // create a function which now serves as the insert point
//   builder.create_ret(...);      // create a return in the given function
//
// The insertion point is set automatically, whenever a new function is created, but, if need
// be, it can be modified using the set_insert_point method. Whilst the module itself can be
// modified using its methods and/or the function methods, I'd generally recommend you to use
// the builder interface, as it simplifies and unifies the respective operations.

namespace sigma::ir {
	class builder {
	public:
		builder(module& target);

		auto create_function(
			const function_signature& function_sig, linkage linkage
		) -> handle<function>;

		auto create_region() const->handle<node>;
		void create_branch(handle<node> target) const;

		void create_conditional_branch(
			handle<node> condition,
			handle<node> if_true,
			handle<node> if_false
		) const;

		auto create_call(
			handle<external> target,
			const function_signature& function_sig,
			const std::vector<handle<node>>& parameters
		) const -> handle<node>;

		auto create_call(
			handle<function> target,
			const std::vector<handle<node>>& parameters
		) const -> handle<node>;

		void create_return(const std::vector<handle<node>>& return_values) const;

		auto create_external(
			const std::string& identifier, 
			linkage linkage
		) const -> handle <external>;

		auto create_external(
			const function_signature& function_sig,
			linkage linkage
		) const -> handle<external>;

		auto create_global(const std::string& identifier, linkage linkage) const -> handle<global>;
		auto create_local(u16 size, u16 alignment) const -> handle<node>;
		auto get_function_parameter(u64 index) const->handle<node>;

		auto create_string(const std::string& value) const -> handle<node>;
		auto create_signed_integer(i64 value, u8 bit_width) const -> handle<node>;
		auto create_bool(bool value) const -> handle<node>;;

		// operators

		auto create_add(
			handle<node> left, 
			handle<node> right,
			arithmetic_behaviour behaviour = arithmetic_behaviour::none
		) const -> handle<node>;

		auto create_sub(
			handle<node> left, 
			handle<node> right, 
			arithmetic_behaviour behaviour = arithmetic_behaviour::none
		) const -> handle<node>;

		auto create_mul(
			handle<node> left,
			handle<node> right,
			arithmetic_behaviour behaviour = arithmetic_behaviour::none
		) const -> handle<node>;

		auto create_load(
			handle<node> value_to_load, 
			data_type data_type,
			u32 alignment, 
			bool is_volatile
		) const->handle<node>;

		void create_store(
			handle<node> destination,
			handle<node> value,
			u32 alignment,
			bool is_volatile
		) const;

		[[nodiscard]] auto get_insert_point() const -> handle<function>;
		void set_insert_point(handle<function> function);

		void set_control(handle<node> control) const;
	private:
		[[nodiscard]] auto get_insert_point_checked() const -> handle<function>;
	private:
		handle<function> m_insert_point;
		module& m_target;
	};
} // namespace sigma::ir
