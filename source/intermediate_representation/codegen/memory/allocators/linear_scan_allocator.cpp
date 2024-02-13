#include "linear_scan_allocator.h"
#include <compiler/compiler/compiler.h>

namespace sigma::ir {
	void linear_scan_allocator::allocate(codegen_context& context) {
		clear();
		m_cache = context.first;
		m_free_positions.resize(16);

		for (u8 i = 0; i < REGISTER_CLASS_COUNT; ++i) {
			m_active_set[i] = utility::dense_set(16);
		}

		const u64 interval_count = context.intervals.size();

		// create intervals 
		for (u64 i = context.basic_block_order.size(); i-- > 0;) {
			handle<node> basic_block = context.work.items[context.basic_block_order[i]];
			const handle machine_block = &context.machine_blocks.at(basic_block);
			const u64 block_start = machine_block->start;
			const u64 black_end = machine_block->end + 2;
			auto& live_out = machine_block->live_out;

			// for anything that's live out, add the entire range
			for (u64 j = 0; j < (interval_count + 63) / 64; ++j) {
				const u64 bits = live_out.data(j);

				if (bits == 0) {
					continue;
				}

				for (u8 k = 0; k < 64; ++k) {
					if (bits & (1ull << k)) {
						context.intervals[j * 64 + k].add_range(
							{ block_start, black_end }
						);
					}
				}
			}

			if (machine_block->first) {
				reverse_block_walk(context, machine_block, machine_block->first);
			}
		}

		// we use every fixed interval at the very start to force them into
		// the inactive set.
		for(u8 i = 0; i < 32; ++i) {
			context.intervals[i].add_range({ 0, 1 });
		}

		mark_callee_saved_constraints(context);

		// generate unhandled interval list (sorted by starting point)
		for(u64 i = 0; i < interval_count; ++i) {
			context.intervals[i].active_range = context.intervals[i].ranges.size() - 1;
			m_unhandled.push_back(i);
		}

		quick_sort_definitions(
			context.intervals, 0, static_cast<ptr_diff>(interval_count) - 1, m_unhandled
		);

		// linear scan main loop
		while(!m_unhandled.empty()) {
			const u64 register_index = m_unhandled.back();

			handle interval = &context.intervals[register_index];
			u64 time = interval->ranges[interval->active_range].start;
			m_unhandled.pop_back();

			ASSERT(time != std::numeric_limits<u64>::max(), "panic");

			if(interval->reg.is_valid() == false && interval->spill > 0) {
				continue;
			}

			// expire intervals
			for(int rc = 0; rc < REGISTER_CLASS_COUNT; ++rc) {
				foreach_set(m_active_set[rc], [&](u64 reg) {
					const ptr_diff active_index = m_active[rc][reg];
					update_interval(context, &context.intervals[active_index], true, time, -1);
				});
			}

			for(u64 i = 0; i < m_inactive.size();) {
				const auto inactive_i = m_inactive[i];
				const auto inactive_it = &context.intervals[inactive_i];

				if(update_interval(context, inactive_it, false, time, i)) {
					interval = &context.intervals[register_index];
					continue;
				}

				i++;
			}

			reg reg = interval->reg;

			if(reg.is_valid() == false) {
				// find register for virtual interval
				reg = allocate_free_reg(context, interval);
				interval = &context.intervals[register_index];

				if(reg.is_valid() == false) {
					reg = allocate_blocked_reg(context, interval);
					interval = &context.intervals[register_index];
				}
			}

			// add the register to the active set
			if (reg.is_valid()) {
				interval->assigned = reg;

				move_to_active(context, interval);
			}
		}

		// move the resolver
		for(const u64 block_order : context.basic_block_order) {
			handle<node> basic_block = context.work.items[block_order];
			auto machine_block = &context.machine_blocks.at(basic_block);
			const auto end_node = machine_block->end_node;

			for(handle<user> use = end_node->use; use; use = use->next_user) {
				if(!use->target->is_control()) {
					continue;
				}

				const handle<node> successor = use->target->get_fallthrough();
				auto target = &context.machine_blocks.at(successor);

				// for all live-ins, we should check if we need to insert a move
				foreach_set(target->live_in, [&](u64 k) {
					const auto interval = &context.intervals[k];

					// if the value changes across the edge, insert a move
					const handle<live_interval> start = interval->split_at(
						context, machine_block->end
					);

					const handle<live_interval> end = interval->split_at(
						context, target->start
					);

					if(start != end) {
						if(start->spill > 0) {
							ASSERT(end->spill <= 0, "both intervals cannot be spilled yet");
							insert_split_move(
								context, 
								target->start + 1, 
								start.get() - context.intervals.data(), 
								end.get() - context.intervals.data()
							);
						}
						else {
							insert_split_move(
								context,
								machine_block->terminator - 1, 
								start.get() - context.intervals.data(),
								end.get() - context.intervals.data()
							);
						}
					}
				});
			}
		}

		// resolve all split interval references
		for(handle<instruction> inst = context.first; inst; inst = inst->next_instruction) {
			if(inst->flags & instruction::SPILL) {
				continue;
			}

			const u64 position = inst->time;
			const u64 rel_count = inst->out_count + inst->in_count + inst->tmp_count + inst->save_count;

			for (u64 i = 0; i < rel_count; ++i) {
				inst->operands[i] = static_cast<i32>(context.intervals[inst->operands[i]].split_at(
					context, position
				).get() - context.intervals.data());
			}
		}

		for (u64 i = 0; i < context.intervals.size(); ++i) {
			context.intervals[i].ranges.clear();
			context.intervals[i].uses.clear();
		}
	}

	void linear_scan_allocator::clear() {
		m_active_set[0].clear();
		m_active_set[1].clear();

		m_unhandled.clear();
		m_free_positions.clear();
		m_inactive.clear();
	}

	void linear_scan_allocator::reverse_block_walk(
		codegen_context& context, handle<machine_block> block, handle<instruction> inst
	) {
		const handle<instruction> next = inst->next_instruction;

		if(next && next != instruction::type::LABEL) {
			reverse_block_walk(context, block, next);
		}

		// mark outputs, inputs and temps
		const u64 time = inst->time;
		auto ops = inst->operands.begin();

		const bool is_call = 
			inst == instruction::type::CALL ||
			inst == instruction::type::SYS_CALL;

		const bool dst_use_reg = 
			inst == instruction::type::IMUL ||
			inst == instruction::type::ZERO ||
			inst->flags & (instruction::MEM | instruction::GLOBAL);

		for(u8 i = 0; i < inst->out_count; ++i) {
			live_interval* interval = &context.intervals[*ops++];

			if(interval->ranges.size() == 1) {
				interval->add_range({ 
					inst->time, inst->time
				});
			}
			else {
				interval->ranges.back().start = inst->time;
			}

			interval->uses.push_back({
				.position = inst->time, .type = dst_use_reg ? use_position::REG : use_position::OUT
			});
		}

		for (u8 i = 0; i < inst->in_count; ++i) {
			live_interval* interval = &context.intervals[*ops++];
			interval->uses.push_back({ .position = time, .type = use_position::REG });
			interval->add_range({ block->start, time });
		}

		for (int i = 0; i < inst->tmp_count; ++i) {
			live_interval* interval = &context.intervals[*ops++];
			interval->add_range({ inst->time, inst->time + 1 });

			if(!is_call) {
				interval->uses.push_back({ .position = inst->time, .type = use_position::REG });
			}
		}

		// safe points don't care about memory or reg, it just needs to be available
		for(u8 i =0; i < inst->save_count; ++i) {
			live_interval* interval = &context.intervals[*ops++];
			interval->uses.push_back({ .position = time, .type = use_position::MEM_OR_REG });
			interval->add_range({ block->start, time });
		}
	}

	void linear_scan_allocator::mark_callee_saved_constraints(const codegen_context& context) {
		const parameter_descriptor descriptor = context.target.get_parameter_descriptor();

		// don't include RBP and RSP registers
		u32 callee_saved_gp_registers = ~descriptor.caller_saved_gpr_count;
		callee_saved_gp_registers &= ~(1u << x64::gpr::RBP);
		callee_saved_gp_registers &= ~(1u << x64::gpr::RSP);
		m_callee_saved[0] = callee_saved_gp_registers;

		m_callee_saved[1] = 0;
		// mark XMM callees
		for(u16 i = descriptor.caller_saved_xmm_count; i < 16; ++i) {
			m_callee_saved[1] |= (1ull << i);
		}
	}

	u64 linear_scan_allocator::partition(
		const std::vector<live_interval>& intervals, 
		ptr_diff lo, 
		ptr_diff hi, 
		std::vector<u64>& arr
	) {
		const u64 pivot = intervals[arr[(hi - lo) / 2 + lo]].get_start(); // middle

		ptr_diff i = lo - 1;
		ptr_diff j = hi + 1;

		for(;;) {
			// move the left index to the right at least once and while the element
			// at the left index is less than the pivot
			do {
				i += 1; 
			} while (intervals[arr[i]].get_start() > pivot);

			// move the right index to the left at least once and while the element
			// at the right index is greater than the pivot
			do {
				j -= 1; 
			} while (intervals[arr[j]].get_start() < pivot);

			// if the indices crossed, return
			if (i >= j) {
				return j;
			}

			// swap the elements at the left and right indices
			std::swap(arr[i], arr[j]);
		}
	}

	void linear_scan_allocator::quick_sort_definitions(
		std::vector<live_interval>& intervals,
		ptr_diff lo, 
		ptr_diff hi, 
		std::vector<u64>& arr
	) {
		if(lo >= 0 && hi >= 0 && lo < hi) {
			// get pivot
			const u64 pivot = partition(intervals, lo, hi, arr);

			// sort both sides
			quick_sort_definitions(
				intervals, lo, static_cast<ptr_diff>(pivot), arr
			);

			quick_sort_definitions(
				intervals, static_cast<ptr_diff>(pivot) + 1, hi, arr
			);
		}
	}

	bool linear_scan_allocator::update_interval(
		const codegen_context& context,
		handle<live_interval> interval,
		bool is_active, 
		u64 time, ptr_diff inactive_index
	) {
		// get to the right range first
		while (interval->ranges[interval->active_range].end <= time) {
			ASSERT(interval->active_range > 0, "");
			interval->active_range -= 1;
		}

		const ptr_diff register_index = interval.get() - context.intervals.data();
		const classified_reg::class_type register_class = interval->reg.cl;

		const u64 hole_end = interval->ranges[interval->active_range].start;
		const bool is_now_active = time >= hole_end;
		const reg reg = interval->assigned;

		if(interval->active_range == 0) {
			if(is_active) {
				m_active_set[register_class].remove(reg.id);
			}
			else {
				m_inactive.erase(m_inactive.begin() + inactive_index);
				return true;
			}
		}
		else if(is_now_active != is_active){
			if(is_now_active) {
				move_to_active(context, interval);
				m_inactive.erase(m_inactive.begin() + inactive_index);
				return true;
			}

			m_active_set[register_class].remove(reg.id);

			m_inactive.push_back(register_index);
		}

		return false;
	}

	void linear_scan_allocator::move_to_active(
		const codegen_context& context, handle<live_interval> interval
	) {
		const classified_reg::class_type register_class = interval->reg.cl;
		const ptr_diff register_index = interval.get() - context.intervals.data();
		const reg register_id = interval->assigned;

		if(m_active_set[register_class].get(register_id.id)) {
			PANIC("intervals should never be forced out");
		}

		ASSERT(register_id.id < 16, "");
		m_active_set[register_class].put(register_id.id);
		m_active[register_class][register_id.id] = register_index;
	}

	void linear_scan_allocator::insert_split_move(
		codegen_context& context, 
		u64 t, 
		ptr_diff old_reg,
		ptr_diff new_reg
	) {
		// invalidate
		if (m_cache->time >= t) {
			m_cache = context.first;
		}

		const i32 data_type = context.intervals[old_reg].data_type;
		handle<instruction> prev = m_cache;
		handle<instruction> inst = prev->next_instruction;

		while(inst != nullptr) {
			if(inst->time > t) {
				m_cache = prev;
				break;
			}

			prev = inst;
			inst = inst->next_instruction;
		}

		// folded spill
		if (
			inst && inst == instruction::type::MOV &&
			inst->flags == instruction::NONE &&
			inst->operands[0] == old_reg
		) {
			inst->operands[0] = static_cast<i32>(new_reg);
			return;
		}

		const handle<instruction> new_inst = context.create_instruction(2);

		new_inst->set_type(instruction::type::MOV);
		new_inst->flags = instruction::SPILL;
		new_inst->data_type = data_type;

		new_inst->out_count = 1;
		new_inst->in_count = 1;

		new_inst->operands[0] = static_cast<i32>(new_reg);
		new_inst->operands[1] = static_cast<i32>(old_reg);

		new_inst->time = prev->time + 1;
		new_inst->next_instruction = prev->next_instruction;
		prev->next_instruction = new_inst;
	}

	u64 linear_scan_allocator::split_intersecting(
		codegen_context& context,
		u64 current_time, 
		u64 pos,
		handle<live_interval> interval,
		bool is_spill
	) {
		if(interval->spill <= 0) {
			// allocate stack slot
			constexpr u8 size = 8;
			context.stack_usage = utility::align(context.stack_usage + size, size);

			// remove from active set
			m_active_set[interval->reg.cl].remove(interval->assigned.id);
		}

		// split lifetime
		live_interval it = *interval;

		if(is_spill) {
			it.spill = static_cast<i32>(context.stack_usage);
		}
		else {
			it.spill = -1;
		}

		it.reg = classified_reg();

		it.assigned = reg();
		ASSERT(it.assigned != 1, "x");

		it.ranges.reserve(4);
		it.uses.clear();
		it.target = nullptr;
		it.split_child = -1;

		ASSERT(interval->split_child < 0, "cannot spill while spilled");

		const ptr_diff old_reg = interval.get() - context.intervals.data();
		const u64 new_reg = context.intervals.size();

		interval->split_child = static_cast<i32>(new_reg);
		it.add_range(utility::range<u64>::max());

		context.intervals.push_back(it);
		interval = &context.intervals[old_reg];

		if(!is_spill) {
			// since the split is starting at pos and pos is at the top of the
			// unhandled list we can push this to the top wit no problem
			// find the position where the new element should be inserted
			const auto unhandled_it = std::ranges::find_if(
				m_unhandled.begin(), m_unhandled.end(), [&](const u64 index) {
					return pos > context.intervals[index].get_start();
				}
			);

			// insert the new element at the found position
			m_unhandled.insert(unhandled_it, new_reg);

			// we know where to insert
		}

		const u64 use_count = interval->uses.size();
		// loop through the uses in reverse

		for (u64 i = use_count; i-- > 0;) {
			const u64 split_count = use_count - (i + 1);

			if (interval->uses[i].position > pos && split_count > 0) {
				// create a new vector for the split uses
				std::vector uses(
					interval->uses.begin() + i + 1, interval->uses.end()
				);

				// resize the original vector
				uses.resize(i + 1);

				// update it uses and interval uses
				it.uses = uses;
				interval->uses = uses;
				break;
			}
		}

		// split ranges
		for (u64 i = 1; i < interval->ranges.size();) {
			utility::range<u64>& interval_range = interval->ranges[i];

			if (interval_range.start > pos) {
				// append the range to it ranges
				it.add_range(interval_range);
				interval = &context.intervals[old_reg];

				// remove and shift
				interval->ranges.erase(interval->ranges.begin() + i);
				continue;
			}

			if (interval_range.end > pos) {
				// intersects pos, we need to split the range
				utility::range<u64> r;
				r.start = pos;
				r.end = interval_range.end;

				interval_range.end = pos;

				// add the new range to it ranges
				it.add_range(r);

				// update the interval
				interval = &context.intervals[old_reg];
				continue;
			}

			++i;
		}

		// insert move (the control flow aware moves are inserted later)
		insert_split_move(context, pos, old_reg, new_reg);

		// reload before next use
		if (is_spill) {
			for(u64 i = it.uses.size(); i -- > 0;) {
				if(it.uses[i].type == use_position::REG) {
					// new split
					split_intersecting(
						context, 
						current_time, 
						it.uses[i].position - 1, 
						&context.intervals[new_reg], 
						false
					);

					break;
				}
			}
		}

		return new_reg;
	}

	reg linear_scan_allocator::allocate_free_reg(codegen_context& context, handle<live_interval> interval) {
		const auto register_class = interval->reg.cl;
		constexpr i32 half_free = 1 << 16;

		for(int i = 0; i < 16; ++i) {
			m_free_positions[i] = 
				(m_callee_saved[register_class] & (1ull << i)) ? half_free : std::numeric_limits<i32>::max();
		}

		// for each active reg, set the free pos to 0
		foreach_set(m_active_set[register_class], [&](u64 i) {
			m_free_positions[i] = 0;
		});

		// for each inactive which intersects current
		for(const ptr_diff i : m_inactive) {
			const auto it = &context.intervals[i];
			const ptr_diff free_position = m_free_positions[it->assigned.id];

			if(free_position > 0) {
				const ptr_diff intersect = interval_intersect(interval, it);
				if(intersect >= 0 && intersect < free_position) {
					m_free_positions[it->assigned.id] = intersect;
				}
			}
		}

		if(register_class == x64::register_class::GPR) {
			// reserved registers
			m_free_positions[static_cast<u8>(x64::gpr::RBP)] = 0;
			m_free_positions[static_cast<u8>(x64::gpr::RSP)] = 0;
		}

		// try hint
		reg highest;

		if(interval->hint.is_valid()) {
			const auto hint = &context.intervals[interval->hint.id];
			ASSERT(hint->reg.cl == register_class, "invalid hint register class");
			const reg hint_reg = hint->assigned;

			if(static_cast<ptr_diff>(interval->get_end()) <= m_free_positions[hint_reg.id]) {
				highest = hint_reg;
			}
		}

		// pick highest free pos
		if(highest.is_valid() == false) {
			 highest.id = 0;

			for(u8 i = 1; i < 16; ++i) {
				if(m_free_positions[i] > m_free_positions[highest.id]) {
					highest = i;
				}
			}
		}

		const ptr_diff free_position = m_free_positions[highest.id];
		if(free_position == 0) {
			return reg::invalid_id;
		}

		if(m_callee_saved[register_class] & (1ull << highest.id)) {
			m_callee_saved[register_class] &= (1ull << highest.id);

			const u8 size = register_class ? 16 : 8;
			u8 virtual_reg = highest.id;

			if(register_class) {
				virtual_reg += x64::register_class::FIRST_XMM;
			}
			else {
				virtual_reg += x64::register_class::FIRST_GPR;
			}

			context.stack_usage = utility::align(context.stack_usage + size, size);

			const live_interval it{
				.reg = classified_reg(),
				.data_type = context.intervals[virtual_reg].data_type,
				.spill = static_cast<i32>(context.stack_usage)
			};

			const ptr_diff old_reg = interval.get() - context.intervals.data();
			const u64 spill_slot = context.intervals.size();
			context.intervals.push_back(it);

			// insert spill and reload
			insert_split_move(context, 0, virtual_reg, spill_slot);

			if(context.endpoint) {
				insert_split_move(context, context.endpoint, spill_slot, virtual_reg);
			}

			interval = &context.intervals[old_reg];
			ASSERT(interval->assigned != 1, "x");
		}

		if(static_cast<ptr_diff>(interval->get_end()) > free_position) {
			interval->assigned = highest;
			split_intersecting(context, interval->get_start(), free_position - 1, interval, true);
		}

		return highest;
	}

	reg linear_scan_allocator::allocate_blocked_reg(
		codegen_context& context, handle<live_interval> interval
	) {
		SUPPRESS_C4100(context);
		SUPPRESS_C4100(interval);

		NOT_IMPLEMENTED();
		return reg();
	}
} // namespace sigma::ir
