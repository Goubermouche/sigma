#include "instruction.h"

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

	phi_value::phi_value(
		handle<node> n, 
		handle<node> phi,
		u8 source,
		u8 destination
	) : m_node(n),
		m_phi(phi), 
		m_source(source),
		m_destination(destination) {}

	void phi_value::set_node(handle<node> node) {
		m_node = node;
	}

	void phi_value::set_phi(handle<node> phi) {
		m_phi = phi;
	}

	void phi_value::set_source(u8 source) {
		m_source = source;
	}

	void phi_value::set_destination(u8 dst) {
		m_destination = dst;
	}

	handle<node> phi_value::get_phi() const {
		return m_phi;
	}

	handle<node> phi_value::get_node() const {
		return m_node;
	}

	u8 phi_value::get_destination() const {
		return m_destination;
	}
}