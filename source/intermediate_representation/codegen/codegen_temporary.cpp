#include "codegen_temporary.h"
#include "intermediate_representation/codegen/codegen_context.h"

namespace sigma::ir {
	auto codegen_temporary::matches(handle<codegen_temporary> b) const -> bool {
		if (type != b->type) {
			return false;
		}

		if (type == mem) {
			return
				reg == b->reg &&
				index == b->index &&
				sc == b->sc;
		}

		return (type == gpr || type == xmm) ? reg == b->reg : false;
	}

	auto codegen_temporary::create_label(codegen_context& context, u64 target) -> handle<codegen_temporary> {
		const handle<codegen_temporary> val = context.create_temporary<sigma::ir::label>();
		val->type = label;
		val->get<sigma::ir::label>().value = target;
		return val;
	}

	auto codegen_temporary::create_imm(codegen_context& context, i32 imm) -> handle<codegen_temporary> {
		const handle<codegen_temporary> val = context.create_temporary();
		val->type = codegen_temporary::imm;
		val->immediate = imm;
		return val;
	}

	auto codegen_temporary::create_abs(codegen_context& context, u64 abs) -> handle<codegen_temporary> {
		const handle<codegen_temporary> val = context.create_temporary<absolute>();
		val->type = codegen_temporary::abs;
		val->get<absolute>().value = abs;
		return val;
	}
}