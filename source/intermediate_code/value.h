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
		 * @address or, in overloaded cases, its value - ie. i32 999 for a 32 bit
		 * signed integer with the value of '999'). 
		 * \return String representation of the stored value. 
		 */
		virtual std::string get_value_string();
	protected:
		std::string m_name;
	};

	struct alignment {
		alignment(u64 value)
			: m_value(value) {}

		u64 get_value() const {
			return m_value;
		}
	private:
		u64 m_value; // alignment, in bytes
	};

	class data_layout {
	public:
		data_layout(u64 preferred_alignment)
			: m_preferred_alignment(preferred_alignment) {}

		u64 get_alignment_for_bit_width(u64 bit_width) const {
			if(bit_width == 0) {
				return 1; // default to 1 byte alignment
			}

			const u64 byte_width = (bit_width + 7) / 8;
			u64 alignment = 1;

			// determine alignment as the next power of two, capped at m_preferred_alignment.
			while (alignment < byte_width && alignment < m_preferred_alignment) {
				alignment *= 2;
			}

			return alignment;
		}
	private:
		u64 m_preferred_alignment;
	};
}