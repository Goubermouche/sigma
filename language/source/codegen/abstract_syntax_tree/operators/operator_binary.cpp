#include "operator_binary.h"

namespace channel {
	operator_binary::operator_binary(u64 line_number, node* left, node* right)
		: node(line_number), left(left), right(right) {}

	operator_binary::~operator_binary() {
		delete left;
		delete right;
	}
}