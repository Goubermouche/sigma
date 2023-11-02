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

	void instruction::set_time(i32 time) {
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

	i32 instruction::get_time() const {
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
		u8 destination,
		u8 left, 
		u8 right
	) {
		const handle<instruction> inst = context.create_instruction<immediate_prop>(
			type, data_type, 1, 2, 0
		);

		inst->set_operand(0, destination);
		inst->set_operand(1, left);
		inst->set_operand(2, right);

		return inst;
	}

	handle<instruction> instruction::create_rri(
		code_generator_context& context, 
		instruction::type type, 
		const data_type& data_type,
		u8 destination, 
		u8 source,
		i32 imm
	) {
		const handle<instruction> inst = context.create_instruction<immediate_prop>(
			type, data_type, 1, 1, 0
		);

		inst->get<immediate_prop>()->value = imm;
		inst->set_flags(instruction::immediate);
		inst->set_operand(0, destination);
		inst->set_operand(1, source);
		return inst;
	}

	handle<instruction> instruction::create_rrm(
		code_generator_context& context, 
		instruction::type type,
		const data_type& data_type, 
		i32 destination,
		i32 source, 
		i32 base, 
		i32 index,
		scale scale,
		i32 displacement
	) {
		const handle<instruction> inst = context.create_instruction<empty_property>(
			type, data_type, 1, index >= 0 ? 3 : 2, 0
		);

		inst->set_flags(instruction::mem | (index >= 0 ? instruction::indexed : instruction::none));
		inst->set_scale(scale);
		inst->set_displacement(displacement);
		inst->set_memory_slot(2);

		inst->set_operand(0, destination);
		inst->set_operand(1, source);
		inst->set_operand(2, base);

		if (index >= 0) {
			inst->set_operand(4, index);
		}

		return inst;
	}

	handle<instruction> instruction::create_rm(
		code_generator_context& context, 
		instruction::type type, 
		const data_type& data_type, 
		i32 destination, 
		i32 base, 
		i32 index,
		scale scale, 
		i32 displacement
	) {
		const handle<instruction> inst = context.create_instruction<empty_property>(
			type, data_type, 1, index >= 0 ? 2 : 1, 0
		);

		inst->set_flags(instruction::mem | (index >= 0 ? instruction::indexed : instruction::none));
		inst->set_memory_slot(1);
		inst->set_operand(0, destination);
		inst->set_operand(1, base);

		if (index >= 0) {
			inst->set_operand(2, index);
		}

		inst->set_displacement(displacement);
		inst->set_scale(scale);
		return inst;
	}

	handle<instruction> instruction::create_mr(
		code_generator_context& context, 
		instruction::type type, 
		const data_type& data_type,
		i32 base,
		i32 index,
		scale scale,
		i32 displacement,
		i32 source
	) {
		const handle<instruction> inst = context.create_instruction<empty_property>(
			type, data_type, 0, index >= 0 ? 3 : 2, 0
		);

		inst->set_flags(instruction::mem | (index >= 0 ? instruction::indexed : instruction::none));
		inst->set_memory_slot(0);

		inst->set_operand(0, base);

		if (index >= 0) {
			inst->set_operand(1, index);
			inst->set_operand(2, source);
		}
		else {
			inst->set_operand(1, source);
		}

		inst->set_displacement(displacement);
		inst->set_scale(scale);
		return inst;
	}

	handle<instruction> instruction::create_move(
		code_generator_context& context, 
		const data_type& data_type, 
		u8 destination,
		u8 source
	) {
		const i32 machine_data_type = context.target->legalize_data_type(data_type);
		const handle<instruction> inst = context.create_instruction<empty_property>(2);

		inst->set_type(machine_data_type >= sse_ss ? instruction::floating_point_mov : instruction::mov);
		inst->set_data_type(machine_data_type);

		inst->set_out_count(1);
		inst->set_in_count(1);
		inst->set_operand(0, destination);
		inst->set_operand(1, source);

		return inst;
	}

	handle<instruction> instruction::create_abs(
		code_generator_context& context, 
		instruction::type type,
		const data_type& data_type,
		u8 destination, 
		u64 imm
	) {
		const handle<instruction> inst = context.create_instruction<immediate_prop>(
			type, data_type, 1, 0, 0
		);

		inst->get<immediate_prop>()->value = static_cast<i32>(imm);
		inst->set_flags(instruction::absolute);
		inst->set_operand(0, destination);
		return inst;
	}

	handle<instruction> instruction::create_zero(
		code_generator_context& context, 
		const data_type& data_type,
		u8 destination
	) {
		const handle<instruction> inst = context.create_instruction<empty_property>(
			instruction::zero, data_type, 1, 0, 0
		);

		inst->set_operand(0, destination);
		return inst;
	}

	handle<instruction> instruction::create_immediate(
		code_generator_context& context, 
		instruction::type type, 
		const data_type& data_type,
		u8 destination, 
		i32 imm
	) {
		const handle<instruction> inst = context.create_instruction<immediate_prop>(
			type, data_type, 1, 0, 0
		);

		inst->get<immediate_prop>()->value = imm;
		inst->set_flags(instruction::immediate);
		inst->set_operand(0, destination);
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
