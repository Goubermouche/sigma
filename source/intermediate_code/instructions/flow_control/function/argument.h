#pragma once
#include "intermediate_code/types/type.h"

namespace ir {
	class function_argument;
	using function_argument_ptr = ptr<function_argument>;

	/**
	 * \brief Function argument type, contains information about a
	 * given function argument.
	 */
	class function_argument : public value {
	public:
		function_argument(type_ptr type, const std::string& name);

		type_ptr get_type() const;
		std::string to_string() override;
	private:
		type_ptr m_type;
	};
}
