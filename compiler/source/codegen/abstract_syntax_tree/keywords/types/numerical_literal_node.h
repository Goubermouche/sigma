#pragma once
#include "codegen/abstract_syntax_tree/node.h"

namespace channel {
	class numerical_literal_node : public node {
	public:
		numerical_literal_node(
			const token_position& position,
			const std::string& value,
			type preferred_type
		);

		acceptation_result accept(
			visitor& visitor,
			const codegen_context& context
		) override;

		void print(u64 depth, const std::wstring& prefix, bool is_last) override;

		const std::string& get_value() const;
		const type& get_preferred_type() const;
	private:
		std::string m_value;
		type m_preferred_type;
	};
}