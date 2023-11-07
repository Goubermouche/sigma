#include "instruction.h"
#include "intermediate_representation/code_generation/code_generator_context.h"
#include "intermediate_representation/code_generation/targets/x64_target.h"

namespace ir::cg {
	void instruction::set_type(type type) {
		m_type = type;
	}

	void instruction::set_flags(flags flags) {
		m_flags = flags;
	}

	void instruction::set_memory_slot(i32 slot) {
		m_memory_slot = slot;
	}

	void instruction::set_displacement(i32 displacement) {
		m_displacement = displacement;
	}

	void instruction::set_scale(scale scale) {
		m_scale = scale;
	}

	void instruction::set_next_instruction(handle<instruction> next) {
		m_next = next;
	}

	void instruction::set_operands(const utility::slice<i32>& operands) {
		m_operands = operands;
	}

	void instruction::set_operand(u64 index, i32 value) {
		m_operands[index] = value;
	}

	void instruction::set_data_type(i32 data_type) {
		m_data_type = data_type;
	}

	void instruction::set_in_count(u8 in_count) {
		m_in_count = in_count;
	}

	void instruction::set_out_count(u8 out_count) {
		m_out_count = out_count;
	}

	void instruction::set_tmp_count(u8 tmp_count) {
		m_tmp_count = tmp_count;
	}

	void instruction::set_time(u64 time) {
		m_time = time;
	}

	handle<instruction> instruction::get_next_instruction() const {
		return m_next;
	}

	utility::slice<i32>& instruction::get_operands() {
		return m_operands;
	}

	i32 instruction::get_operand(u64 index) const {
		return m_operands[index];
	}

	const utility::slice<i32>& instruction::get_operands() const {
		return m_operands;
	}

	u8 instruction::get_in_count() const {
		return m_in_count;
	}

	u8 instruction::get_out_count() const {
		return m_out_count;
	} 

	u8 instruction::get_tmp_count() const {
		return m_tmp_count;
	}

	u16 instruction::get_operand_count() const {
		return m_in_count + m_out_count + m_tmp_count;
	}

	instruction::flags instruction::get_flags() const {
		return m_flags;
	}

	instruction::type instruction::get_type() const {
		return m_type;
	}

	u64 instruction::get_time() const {
		return m_time;
	}

	handle<instruction> instruction::create_label(
		code_generator_context& context, handle<node> target
	) {
		const handle<instruction> inst = context.create_instruction<node_prop>(0);
		inst->get<node_prop>()->value = target;
		inst->set_type(instruction::label);
		inst->set_flags(instruction::node_f);
		return inst;
	}

	handle<instruction> instruction::create_jump(
		code_generator_context& context,
		handle<node> target
	) {
		ASSERT(
			target->get_type() > ir::node::none,
			"invalid target type for a jump instruction"
		);

		const handle<instruction> inst = context.create_instruction<node_prop>(
			instruction::jmp, VOID_TYPE, 0, 0, 0
		);

		inst->get<node_prop>()->value = target;
		inst->set_flags(instruction::node_f);
		return inst;
	}

	handle<instruction> instruction::create_rrr(
		code_generator_context& context, 
		instruction::type type, 
		const data_type& data_type, 
		reg destination,
		reg left, 
		reg right
	) {
		const handle<instruction> inst = context.create_instruction<immediate_prop>(
			type, data_type, 1, 2, 0
		);

		inst->set_operand(0, destination.get_id());
		inst->set_operand(1, left.get_id());
		inst->set_operand(2, right.get_id());

		return inst;
	}

	handle<instruction> instruction::create_rri(
		code_generator_context& context, 
		instruction::type type, 
		const data_type& data_type,
		reg destination, 
		reg source,
		i32 immediate
	) {
		const handle<instruction> inst = context.create_instruction<immediate_prop>(
			type, data_type, 1, 1, 0
		);

		inst->get<immediate_prop>()->value = immediate;
		inst->set_flags(instruction::immediate);
		inst->set_operand(0, destination.get_id());
		inst->set_operand(1, source.get_id());
		return inst;
	}

	handle<instruction> instruction::create_rrm(
		code_generator_context& context, 
		instruction::type type,
		const data_type& data_type, 
		reg destination,
		reg source, 
		reg base, 
		mem memory
	) {
		const handle<instruction> inst = context.create_instruction<empty_property>(
			type, data_type, 1, memory.get_index() >= 0 ? 3 : 2, 0
		);

		inst->set_flags(instruction::mem_f | (memory.get_index() >= 0 ? instruction::indexed : instruction::none));
		inst->set_scale(memory.get_scale());
		inst->set_displacement(memory.get_displacement());
		inst->set_memory_slot(2);

		inst->set_operand(0, destination.get_id());
		inst->set_operand(1, source.get_id());
		inst->set_operand(2, base.get_id());

		if (memory.get_index() >= 0) {
			inst->set_operand(4, memory.get_index());
		}

		return inst;
	}

	handle<instruction> instruction::create_rm(
		code_generator_context& context, 
		instruction::type type, 
		const data_type& data_type, 
		reg destination, 
		reg base, 
		mem memory
	) {
		const handle<instruction> inst = context.create_instruction<empty_property>(
			type, data_type, 1, memory.get_index() >= 0 ? 2 : 1, 0
		);

		inst->set_flags(instruction::mem_f | (memory.get_index() >= 0 ? instruction::indexed : instruction::none));
		inst->set_memory_slot(1);
		inst->set_operand(0, destination.get_id());
		inst->set_operand(1, base.get_id());

		if (memory.get_index() >= 0) {
			inst->set_operand(2, memory.get_index());
		}

		inst->set_displacement(memory.get_displacement());
		inst->set_scale(memory.get_scale());
		return inst;
	}

	handle<instruction> instruction::create_mr(
		code_generator_context& context, 
		instruction::type type, 
		const data_type& data_type,
		reg base,
		mem memory,
		i32 source
	) {
		const handle<instruction> inst = context.create_instruction<empty_property>(
			type, data_type, 0, memory.get_index() >= 0 ? 3 : 2, 0
		);

		inst->set_flags(instruction::mem_f | (memory.get_index() >= 0 ? instruction::indexed : instruction::none));
		inst->set_memory_slot(0);

		inst->set_operand(0, base.get_id());

		if (memory.get_index() >= 0) {
			inst->set_operand(1, memory.get_index());
			inst->set_operand(2, source);
		}
		else {
			inst->set_operand(1, source);
		}

		inst->set_displacement(memory.get_displacement());
		inst->set_scale(memory.get_scale());
		return inst;
	}

	handle<instruction> instruction::create_move(
		code_generator_context& context, 
		const data_type& data_type, 
		reg destination,
		reg source
	) {
		const i32 machine_data_type = context.target->legalize_data_type(data_type);
		const handle<instruction> inst = context.create_instruction<empty_property>(2);

		inst->set_type(machine_data_type >= sse_ss ? instruction::floating_point_mov : instruction::mov);
		inst->set_data_type(machine_data_type);

		inst->set_out_count(1);
		inst->set_in_count(1);
		inst->set_operand(0, destination.get_id());
		inst->set_operand(1, source.get_id());

		return inst;
	}

	handle<instruction> instruction::create_abs(
		code_generator_context& context, 
		instruction::type type,
		const data_type& data_type,
		reg destination,
		u64 immediate
	) {
		const handle<instruction> inst = context.create_instruction<immediate_prop>(
			type, data_type, 1, 0, 0
		);

		inst->get<immediate_prop>()->value = static_cast<i32>(immediate);
		inst->set_flags(instruction::absolute);
		inst->set_operand(0, destination.get_id());
		return inst;
	}

	handle<instruction> instruction::create_zero(
		code_generator_context& context, 
		const data_type& data_type,
		reg destination
	) {
		const handle<instruction> inst = context.create_instruction<empty_property>(
			instruction::zero, data_type, 1, 0, 0
		);

		inst->set_operand(0, destination.get_id());
		return inst;
	}

	handle<instruction> instruction::create_immediate(
		code_generator_context& context, 
		instruction::type type, 
		const data_type& data_type,
		reg destination,
		i32 immediate
	) {
		const handle<instruction> inst = context.create_instruction<immediate_prop>(
			type, data_type, 1, 0, 0
		);

		inst->get<immediate_prop>()->value = immediate;
		inst->set_flags(instruction::immediate);
		inst->set_operand(0, destination.get_id());
		return inst;
	}

	i32 instruction::get_data_type() const {
		return m_data_type;
	}

	scale instruction::get_scale() const {
		return m_scale;
	}

	i32 instruction::get_displacement() const {
		return m_displacement;
	}

	i32 instruction::get_memory_slot() const {
		return m_memory_slot;
	}

	bool instruction::is_terminator() const {
		return m_type == terminator || m_type == INT3 || m_type == UD2;
	}
}
