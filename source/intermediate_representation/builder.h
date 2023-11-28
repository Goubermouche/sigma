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

namespace ir {
	class builder {
	public:
		builder(module& target);

		auto create_function(const function_type& type, linkage linkage) -> handle<function>;

		auto create_call(handle<external> target, const function_type& type, const std::vector<handle<node>>& parameters) const -> std::vector<handle<node>>;
		auto create_call(handle<function> target, const std::vector<handle<node>>& parameters) const-> std::vector<handle<node>>;
		void create_return(const std::vector<handle<node>>& return_values) const;

		auto get_parameter(u64 index) const-> handle<node>;

		auto create_external(const std::string& identifier, linkage linkage) const -> handle <external>;
		auto create_external(const function_type& type, linkage linkage) const -> handle<external>;

		auto create_global(const std::string& identifier, linkage linkage) const -> handle<global>;
		auto create_local(u16 size, u16 alignment) const -> handle<node>;

		auto create_string(const std::string& value) const -> handle<node>;
		auto create_signed_integer(i64 value, u8 bit_width) const -> handle<node>;

		auto create_add(handle<node> left, handle<node> right, arithmetic_behaviour behaviour = arithmetic_behaviour::none) const -> handle<node>;

		[[nodiscard]] auto get_insert_point() const -> handle<function>;
		void set_insert_point(handle<function> function);
	private:
		[[nodiscard]] auto get_insert_point_checked() const -> handle<function>;
	private:
		handle<function> m_insert_point;
		module& m_target;
	};
}
