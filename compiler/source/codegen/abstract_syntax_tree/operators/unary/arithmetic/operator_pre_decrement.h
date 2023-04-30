#pragma once
#include "../operator_unary.h"

namespace channel {
	class operator_pre_decrement : public operator_unary {
	public:
		operator_pre_decrement(u64 line_number, node_ptr expression_node);
		bool accept(visitor& visitor, value_ptr& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;
	};
}