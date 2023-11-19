#include "value.h"
#include "intermediate_representation/codegen/codegen_context.h"

namespace ir {
	auto value::matches(handle<value> b) const -> bool {
		if (m_type != b->m_type) {
			return false;
		}

		if (m_type == mem) {
			return
				m_reg == b->m_reg &&
				m_index == b->m_index &&
				m_scale == b->m_scale;
		}

		return (m_type == gpr || m_type == xmm) ? m_reg == b->m_reg : false;
	}

	auto value::create_label(codegen_context& context, u64 target) -> handle<value> {
		const handle<value> val = context.create_value<ir::label>();
		val->m_type = label;
		val->get<ir::label>().value = target;
		return val;
	}

	auto value::create_imm(codegen_context& context, i32 imm) -> handle<value> {
		const handle<value> val = context.create_value();
		val->m_type = type::imm;
		val->m_imm = imm;
		return val;
	}

	auto value::create_abs(codegen_context& context, u64 abs) -> handle<value> {
		const handle<value> val = context.create_value<absolute>();
		val->m_type = type::abs;
		val->get<absolute>().value = abs;
		return val;
	}
}