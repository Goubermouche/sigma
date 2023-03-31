#include "operator_binary.h"

namespace channel {
	operator_binary::operator_binary(node* left, node* right)
		: left(left), right(right) {}

	operator_binary::~operator_binary() {
		delete left;
		delete right;
	}
}