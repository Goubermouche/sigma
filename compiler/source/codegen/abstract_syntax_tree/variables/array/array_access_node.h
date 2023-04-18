#pragma once
#include "../../node.h"

namespace channel {
	class array_access_node : public node {
	public:
		array_access_node(u64 line_number, const std::string& array_identifier, node* array_index);
		bool accept(visitor& visitor, value*& out_value) override;
		std::string get_node_name() const override;

		const std::string& get_array_identifier() const;
		node* get_array_index_node() const;
	private:
		std::string m_array_identifier;
		node* m_array_index;
	};
}
