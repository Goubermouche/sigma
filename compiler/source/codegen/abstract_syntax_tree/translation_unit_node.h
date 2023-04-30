#pragma once
#include "node.h"

namespace channel {
	class translation_unit_node : public node {
	public:
		translation_unit_node(const std::vector<node*>& nodes);
		bool accept(visitor& visitor, value*& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;

		const std::vector<node*>& get_nodes() const;
	private:
		std::vector<node*> m_nodes;
	};
}
