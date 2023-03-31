#pragma once
#include "../node.h"

namespace channel {
	class operator_binary : public node	{
    public:
        operator_binary(node* left, node* right);
        ~operator_binary();

        node* left;
        node* right;
	};
}

