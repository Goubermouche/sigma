#pragma once
#include "../operator_unary.h"

namespace channel {
	class operator_post_increment : public operator_unary {
	public:
		operator_post_increment(u64 line_number, node* expression_node);
		bool accept(visitor& visitor, value*& out_value) override;
		std::string get_node_name() const override;
	};
}
