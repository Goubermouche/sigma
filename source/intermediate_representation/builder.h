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

		/**
		 * \brief Declares a new function using the provided \b signature and \b linkage.
		 * \param signature Function signature to use
		 * \param linkage Function linkage to use
		 * \return Newly created function.
		 */
		auto create_function(const function_signature& signature, linkage linkage) -> handle<function>;
		
		/**
		 * \brief Creates a new instruction region.
		 * \return Newly created instruction region.
		 */
		auto create_region() const->handle<node>;

		/**
		 * \brief Creates a new branch.
		 * \param target Target region the branch should jump to
		 */
		void create_branch(handle<node> target) const;

		/**
		 * \brief Creates a new conditional branch.
		 * \param condition Condition which determines where the branch jumps to
		 * \param if_true Region to jump to if \b condition == true
		 * \param if_false Region to jump to if \b condition == false
		 */
		void create_conditional_branch(handle<node> condition, handle<node> if_true, handle<node> if_false) const;

		/**
		 * \brief Creates a new function call, which calls an externally declared function.
		 * \param target External to call
		 * \param signature Signature of the external symbol
		 * \param parameters Parameters to call the function with
		 * \return Node containing the result of the function call.
		 */
		auto create_call(handle<external> target, const function_signature& signature, const std::vector<handle<node>>& parameters) const -> handle<node>;

		/**
		 * \brief Creates a new function call, which calls a locally declared function.
		 * \param target Function to call
		 * \param parameters Parameters to call the function with
		 * \return Node containing the result of the function call.
		 */
		auto create_call(handle<function> target, const std::vector<handle<node>>& parameters) const -> handle<node>;

		/**
		 * \brief Creates a new return statement.
		 * \param return_values List of values to return (NOTE: just one return value is supported right now)
		 */
		void create_return(const std::vector<handle<node>>& return_values) const;

		/**
		 * \brief Declares a new external symbol.
		 * \param identifier Identifier of the symbol
		 * \param linkage Linkage of the symbol
		 * \return Newly created external.
		 */
		auto create_external(const std::string& identifier, linkage linkage) const -> handle <external>;

		/**
		 * \brief Declares a new external function symbol.
		 * \param signature Function signature of the external
		 * \param linkage Linkage of the symbol
		 * \return Newly created external function.
		 */
		auto create_external(const function_signature& signature, linkage linkage) const -> handle<external>;

		/**
		 * \brief Creates a new global value.
		 * \param identifier Identifier of the value
		 * \param linkage Linkage of the global
		 * \return Newly created global.
		 */
		auto create_global(const std::string& identifier, linkage linkage) const -> handle<global>;

		/**
		 * \brief Creates a new local value.
		 * \param size Size of the value in bytes
		 * \param alignment Alignment of the value in bytes
		 * \return Newly created local.
		 */
		auto create_local(u16 size, u16 alignment) const -> handle<node>;

		/**
		 * \brief Creates a reference to a given function parameter (serves as a way to access function
		 * parameters from within the context of the current function).
		 * \param index Index of the function parameter
		 * \return Handle to the function respective function parameter.
		 */
		auto get_function_parameter(u64 index) const->handle<node>;

		/**
		 * \brief Creates a new string value.
		 * \param value Value of the string
		 * \return Handle to a node representing the string
		 */
		auto create_string(const std::string& value) const -> handle<node>;

		/**
		 * \brief Creates a new signed integer value.
		 * \param value Value of the signed integer
		 * \param bit_width Bit width of the integer
		 * \return Newly created integer value.
		 */
		auto create_signed_integer(i64 value, u8 bit_width) const -> handle<node>;

		/**
		 * \brief Creates a new boolean value.
		 * \param value Value of the boolean
		 * \return Newly created boolean value.
		 */
		auto create_bool(bool value) const -> handle<node>;

		// operators
		auto create_add(handle<node> left, handle<node> right, arithmetic_behaviour behaviour = arithmetic_behaviour::NONE) const -> handle<node>;
		auto create_sub(handle<node> left, handle<node> right, arithmetic_behaviour behaviour = arithmetic_behaviour::NONE) const -> handle<node>;
		auto create_mul(handle<node> left, handle<node> right, arithmetic_behaviour behaviour = arithmetic_behaviour::NONE) const -> handle<node>;

		/**
		 * \brief Creates a new load operation. 
		 * \param value_to_load Value to be loaded (local / global)
		 * \param data_type Data type to load the value as
		 * \param alignment Alignment of the load operation in bytes
		 * \param is_volatile Whether the load operation is volatile (READ / LOAD)
		 * \return Result of the load operation (loaded value).
		 */
		auto create_load(handle<node> value_to_load, data_type data_type, u32 alignment, bool is_volatile) const->handle<node>;

		/**
		 * \brief Creates a new store operation.
		 * \param destination Destination of the store operation
		 * \param value Value to store at the specified destination
		 * \param alignment Alignment of the store operation in bytes
		 * \param is_volatile Whether the load operation is volatile (WRITE / STORE)
		 */
		void create_store(handle<node> destination, handle<node> value, u32 alignment, bool is_volatile) const;

		/**
		 * \brief Retrieves the current insert point.
		 * \return Current insert point.
		 */
		[[nodiscard]] auto get_insert_point() const -> handle<function>;

		/**
		 * \brief Updates the current insert point.
		 * \param function New insert point
		 */
		void set_insert_point(handle<function> function);

		/**
		 * \brief Sets a new control node (region).
		 * \param control New control node
		 */
		void set_control(handle<node> control) const;
	private:
		[[nodiscard]] auto get_insert_point_checked() const -> handle<function>;
	private:
		handle<function> m_insert_point;
		module& m_target;
	};
} // namespace sigma::ir
