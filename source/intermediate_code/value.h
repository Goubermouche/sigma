// Value is the root class of the intermediate codegen system. Basically
// all other classes in the IR section derive from it, the simplified
// dependency tree can be seen below:
// 
//                   +--------+
//               +-->| type.h |--> ...
//               |   +--------+
//               |
//  +---------+  |   +------------+
//  | value.h +--+-->| constant.h |--> ...
//  +---------+  |   +------------+
//               |
//               |   +---------------+
//               +-->| instruction.h |--> ...
//                   +---------------+

#pragma once
#include <utility/macros.h>

using namespace utility::types;

namespace ir {
	class value;
	using value_ptr = ptr<value>;

	/**
	 * \brief Base of the intermediate codegen system, most classes in this
	 * system also derive from it. Contains basic utility methods and
	 * virtual method declarations used by other child classes. 
	 */
	class value {
	public:
		virtual ~value() = default;

		value(const std::string& name);
		value(const value& other);
		value(value&& other) noexcept;

		value& operator=(const value& other);
		value& operator=(value&& other) noexcept;

		/**
		 * \brief Gets the name of the value, can potentially be empty. 
		 * \return Name of the value
		 */
		const std::string& get_name() const;

		// debugging methods:

		/**
		 * \brief Utility method - converts the value into its string representation. 
		 * \return String representation of the given value.
		 */
		virtual std::string to_string();

		/**
		 * \brief Utility method - converts the value into a value type (by default its
		 * @address or, in overloaded cases, its value - ie. i32 999 for a 32 bit signed
		 * integer with the value of '999'). 
		 * \return String representation of the stored value. 
		 */
		virtual std::string get_value_string();
	protected:
		std::string m_name;
	};
}