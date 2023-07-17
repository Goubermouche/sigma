#pragma once
#include "llvm_wrappers/value.h"

namespace sigma {
	class variable {
	public:
		variable(
			value_ptr value,
			file_position position
		);

		value_ptr get_value() const;
		const file_position& get_position() const;
	private:
		value_ptr m_value;
		file_position m_position;
	};

	using variable_ptr = std::shared_ptr<variable>;
}
