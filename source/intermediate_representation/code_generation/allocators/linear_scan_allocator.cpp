#include "linear_scan_allocator.h"

namespace ir::cg {
	void linear_scan_allocator::allocate(
		code_generator_context& context
	) {
		clear();
		m_cache = context.first;

		for(u64 i = 0; i < REGISTER_CLASS_COUNT; ++i) {
			m_active_set[i] = utility::dense_set(16);
		}

		// build intervals & track when uses happen to aid in splitting
		const u64 interval_count = context.intervals.size();

		for(u64 i = 0; i < context.block_count; ++i) {
			handle<node> block = context.work_list.get_item(i);
			machine_block& machine_block = context.machine_blocks[block];
			i32 block_start = machine_block.get_start();
			i32 block_end = machine_block.get_end();

			// for anything that's live out, add the entire range
			utility::dense_set& live_out = machine_block.get_live_out();

			for(u64 j = 0; j < (interval_count + 63) / 64; ++j) {
				const u64 bits = live_out.data(j);

				if(bits == 0) {
					continue;
				}

				for(u64 k = 0; k < 64; ++k) {
					if(bits & (1ull << k)) {
						context.intervals[j * 64 + k].add_range({
							block_start, block_end
						});
					}
				}
			}

			// for all instruction in the block, create new ranges
			if(machine_block.get_first()) {
				reverse_block_walk(
					context, machine_block, machine_block.get_first()
				);
			}
		}

		// we use every fixed interval at the very start to force them into
		// the inactive set
		for(u64 i = 0; i < 32; ++i) {
			live_interval& interval = context.intervals[i];

			if(!interval.get_ranges().empty()) {
				interval.set_start(0);
				interval.add_range({ 0, 1 });
			}
		}

		// ignore RBP and RSP registers
		context.intervals[static_cast<i32>(gpr::rbp)].get_ranges().clear();
		context.intervals[static_cast<i32>(gpr::rsp)].get_ranges().clear();

		mark_callee_saved_constraints();

		// generate unhandled interval list (sorted by starting point)
		m_unhandled.reserve((interval_count * 4) / 3);
		for(u64 i = 0; i < interval_count; ++i) {
			m_unhandled.push_back(static_cast<i32>(i));
		}

		quick_sort_definitions(
			context.intervals, 0, static_cast<ptr_diff>(interval_count) - 1,
			m_unhandled
		);

		// only need enough to store for the biggest register class
		m_free_positions.reserve(16);
		m_block_positions.reserve(16);

		// linear scan main loop
		while(!m_unhandled.empty()) {
			const i32 register_index = m_unhandled.back();
			live_interval* interval = &context.intervals[register_index];
			m_unhandled.pop_back();

			// unused interval, skip
			if(interval->get_ranges().empty()) {
				continue;
			}

			i32 time = interval->get_start();

			if(interval->get_register().get_id() >= 0) {}
			else if(interval->get_spill() > 0) { continue; }
			else {}

			// expire intervals
			for(u64 register_class = 0; register_class < REGISTER_CLASS_COUNT; ++register_class) {
				foreach_set(m_active_set[register_class], [&](u64 reg) {
					const i32 active_i = m_active[register_class][reg];

					update_interval(
						context, &context.intervals[active_i], true, time, -1
					);
				});
			}

			for(u64 i = 0; i < m_inactive.size();) {
				const i32 inactive_i = m_inactive[i];
				live_interval* inactive_interval = &context.intervals[inactive_i];

				if(update_interval(
					context, inactive_interval, false, time, static_cast<i32>(i))
				) {
					interval = &context.intervals[register_index];
					continue;
				}

				++i;
			}

			ptr_diff reg = interval->get_register().get_id();
			if(reg < 0) {
				// find a register for the virtual interval
				if(reg < 0) {
					reg = allocate_free_reg(context, interval);
					interval = &context.intervals[register_index];
				}

				// allocation failure
				if(reg < 0) {
					reg = allocate_blocked_reg(context, interval);
					interval = &context.intervals[register_index];
				}
			}

			// add to active set
			if(reg >= 0) {
				interval->set_assigned(static_cast<i32>(reg));
				interval->set_active_range(static_cast<i32>(interval->get_ranges().size()) - 1);
				move_to_active(context, interval);
			}
		}

		// resolve moves
		for(u64 i = 0; i < context.block_count; ++i) {
			handle<node> block = context.work_list.get_item(i);

			ASSERT(
				block->get_type() == node::entry || block->get_type() == node::region,
				"node is not a block"
			);

			machine_block& main_machine_block = context.machine_blocks[block];
			const handle<region_property> region = block->get<region_property>();

			if(region->end->get_type() != node::branch) {
				continue;
			}

			const handle<branch_property> branch = region->end->get<branch_property>();

			// iterate over all successor nodes
			for(const handle<node> successor : branch->successors) {
				machine_block& target_block = context.machine_blocks[successor];

				// check if we need to insert a move
				foreach_set(target_block.get_live_in(), [&](u64 live) {
					live_interval* interval = &context.intervals[live];

					// if the value changes across the edge, insert a move
					const live_interval* start = split_interval_at(
						context, interval, main_machine_block.get_end()
					);

					const live_interval* end = split_interval_at(
						context, interval, target_block.get_start()
					);

					if(start != end) {
						if(start->get_spill() > 0) {
							ASSERT(
								end->get_spill() <= 0,
								"two spills at once are not supported yet"
							);

							insert_split_move(
								context,
								target_block.get_start() + 1,
								start - context.intervals.data(),
								end - context.intervals.data()
							);
						}
						else {
							insert_split_move(
								context,
								main_machine_block.get_terminator() - 1,
								start - context.intervals.data(),
								end - context.intervals.data()
							);
						}
					}
				});
			}
		}

		// resolve all split interval references
		for (handle<instruction> inst = context.first; inst; inst =  inst->get_next_instruction()) {
			if(inst->get_flags() & instruction::spill) {
				continue;
			}

			const i32 pos = inst->get_time();
			for(i32& operand : inst->get_operands()) {
				operand = split_interval_at(
					context, &context.intervals[operand], pos
				) - context.intervals.data();
			}
		}

		// free intervals
		for(live_interval& interval : context.intervals) {
			interval.get_ranges().clear();
			interval.get_uses().clear();
		}
	}

	void linear_scan_allocator::clear() {
		m_active_set[0].clear();
		m_active_set[1].clear();

		m_unhandled.clear();
		m_free_positions.clear();
		m_block_positions.clear();
		m_inactive.clear();
	}

	void linear_scan_allocator::reverse_block_walk(
		code_generator_context& context, machine_block& block, handle<instruction> inst
	) {
		const handle<instruction> next = inst->get_next_instruction();
		if(next && next->get_type() != instruction::label) {
			reverse_block_walk(context, block, next);
		}

		// mark outputs, inputs and temporaries
		auto ops = inst->get_operands().begin();
		const bool dst_use_reg =
			inst->get_type() == instruction::integral_multiplication ||
			inst->get_type() == instruction::zero ||
			inst->get_flags() & (instruction::mem | instruction::global);

		for(u8 i = 0; i < inst->get_out_count(); ++i) {
			live_interval& interval = context.intervals[*ops++];

			if(interval.get_ranges().empty()) {
				interval.add_range({ inst->get_time(), inst->get_time() });
			}
			else {
				interval.get_ranges().back().set_start(inst->get_time());
			}

			interval.set_start(inst->get_time());
			interval.add_use_position({ inst->get_time(), dst_use_reg ? use_position::reg : use_position::out });
		}

		for (u8 i = 0; i < inst->get_in_count(); ++i) {
			live_interval& interval = context.intervals[*ops++];

			interval.add_range({ block.get_start(), inst->get_time() });
			interval.add_use_position({ inst->get_time(), use_position::reg });
		}

		const bool is_call =
			inst->get_type() == instruction::call ||
			inst->get_type() == instruction::system_call;

		for (u8 i = 0; i < inst->get_tmp_count(); ++i) {
			live_interval& interval = context.intervals[*ops++];

			interval.add_range({ inst->get_time(), inst->get_time() + 1 });
			if(!is_call) {
				interval.add_use_position({ inst->get_time(), use_position::reg });
			}
		}
	}

	void linear_scan_allocator::mark_callee_saved_constraints() {
		const parameter_descriptor& descriptor = g_parameter_descriptors[0];

		// don't include RBP and RSP registers
		u32 callee_saved_gp_registers = ~descriptor.caller_saved_gpr_count;
		callee_saved_gp_registers &= ~(1u << static_cast<i32>(gpr::rbp));
		callee_saved_gp_registers &= ~(1u << static_cast<i32>(gpr::rsp));
		m_callee_saved[0] = callee_saved_gp_registers;

		// mark XMM callees
		for(u16 i = descriptor.caller_saved_xmm_count; i < 16; ++i) {
			m_callee_saved[1] |= (1ull << i);
		}
	}

	u64 linear_scan_allocator::partition(
		const std::vector<live_interval>& intervals,
		ptr_diff lo, 
		ptr_diff hi,
		std::vector<i32>& arr
	) const {
		const i32 pivot = intervals[arr[(hi - lo) / 2 + lo]].get_start(); // middle

		ptr_diff i = lo - 1;
		ptr_diff j = hi + 1;

		for(;;) {
			// move the left index to the right at least once and while the element
			// at the left index is less than the pivot
			do { i += 1; } while (intervals[arr[i]].get_start() > pivot);

			// move the right index to the left at least once and while the element
			// at the right index is greater than the pivot
			do { j -= 1; } while (intervals[arr[j]].get_start() < pivot);

			// if the indices crossed, return
			if (i >= j) return j;

			// swap the elements at the left and right indices
			std::swap(arr[i], arr[j]);
		}
	}

	void linear_scan_allocator::quick_sort_definitions(
		std::vector<live_interval>& intervals, 
		ptr_diff lo,
		ptr_diff hi, 
		std::vector<i32>& arr
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
		const code_generator_context& context,
		live_interval* interval, 
		bool is_active,
		i32 time,
		i32 inactive_index
	) {
		const i32 register_index = interval - context.intervals.data();

		// get to the right range first
		while(interval->get_range(interval->get_active_range()).get_end() <= time) {
			interval->decrement_active_range();
		}

		const i32 hole_end = interval->get_range(interval->get_active_range()).get_start();
		const i32 register_class = interval->get_register().get_class();
		const i32 register_id = interval->get_assigned();
		const bool is_currently_active = time >= hole_end;

		if(time >= interval->get_end()) {
			// expired
			if(is_active) {
				m_active_set[register_class].remove(register_id);
			}
			else {
				m_inactive.erase(m_inactive.begin() + inactive_index);
				return true;
			}
		}
		else if(is_currently_active != is_active) {
			// if we moved, change which list we're in
			if(is_currently_active) {
				// inactive -> active
				move_to_active(context, interval);
				return true;
			}

			// active -> inactive
			m_active_set[register_class].remove(register_id);
			m_inactive.push_back(register_index);
		}

		return false;
	}

	void linear_scan_allocator::move_to_active(
		const code_generator_context& context, live_interval* interval
	) {
		const i32 register_class = interval->get_register().get_class();
		const i32 register_index = interval - context.intervals.data();
		const i32 register_id = interval->get_assigned();

		if(m_active_set[register_class].get(register_id)) {
			ASSERT(false, "intervals should never be forced out");
		}

		m_active[register_class][register_id] = register_index;
		m_active_set[register_class].put(register_id);
	}

	i32 linear_scan_allocator::range_intersect(i32 start, i32 end, range& b) {
		if(b.get_start() <= end && start <= b.get_end()) {
			return start > b.get_start() ? start : b.get_start();
		}

		return -1;
	}

	void linear_scan_allocator::insert_split_move(
		code_generator_context& context, i32 t, i32 old_reg, i32 new_reg
	) {
		// invalidate
		if (m_cache->get_time() >= t) {
			m_cache = context.first;
		}

		const i32 data_type = context.intervals[old_reg].get_data_type();
		handle<instruction> prev = m_cache;
		handle<instruction> inst = prev->get_next_instruction();

		while(inst != nullptr) {
			if(inst->get_time() > t) {
				m_cache = prev;
				break;
			}

			prev = inst;
			inst = inst->get_next_instruction();
		}

		// folded spill
		if (
			inst && inst->get_type() == instruction::mov &&
			inst->get_flags() == instruction::none &&
			inst->get_operand(0) == old_reg
		) {
			inst->set_operand(0, new_reg);
			return;
		}

		const handle<instruction> new_inst = context.create_instruction<empty_property>(2);

		new_inst->set_type(instruction::mov);
		new_inst->set_flags(instruction::spill);
		new_inst->set_data_type(data_type);

		new_inst->set_out_count(1);
		new_inst->set_in_count(1);

		new_inst->set_operand(0, new_reg);
		new_inst->set_operand(1, old_reg);

		new_inst->set_time(prev->get_time() + 1);
		new_inst->set_next_instruction(prev->get_next_instruction());
		prev->set_next_instruction(new_inst);
	}

	i32 linear_scan_allocator::split_intersecting(
		code_generator_context& context,
		i32 current_time,
		i32 pos, 
		live_interval* interval, 
		bool is_spill
	) {
		ASSERT(interval->get_register().get_id() < 0, "invalid register");

		const i32 register_index = interval - context.intervals.data();

		if(interval->get_spill() > 0) {}
		else {
			// allocate a stack slot
			constexpr u64 size = 8;
			context.stack_usage = utility::align(context.stack_usage + size, size);

			if(current_time >= pos && interval->get_assigned() >= 0) {
				const i32 register_class = interval->get_register().get_class();

				if(m_active_set[register_class].get(interval->get_assigned())) {
					if(m_active[register_class][interval->get_assigned()] == register_index) {
						m_active_set[register_class].remove(interval->get_assigned());
					}
				}
			}
		}

		// split the lifetime of our interval
		live_interval new_interval = *interval;

		if(is_spill) {
			new_interval.set_spill(static_cast<i32>(context.stack_usage));
		}
		else {
			new_interval.set_spill(-1);
		}

		new_interval.get_register().set_id(-1);
		new_interval.set_assigned(-1);
		new_interval.set_start(pos);
		new_interval.set_end(interval->get_end());
		new_interval.set_split_kid(-1);
		interval->set_end(pos);

		const u64 new_register_index = context.intervals.size();
		interval->set_split_kid(static_cast<i32>(new_register_index));

		context.intervals.push_back(new_interval);
		interval = &context.intervals[register_index];

		if(!is_spill) {
			// since the split is starting at pos and pos is at the top of the
			// unhandled list we can push this to the top wit no problem
			// find the position where the new element should be inserted
			const auto it = std::ranges::find_if(
				m_unhandled.begin(), m_unhandled.end(), [&](const i32 index) {
					return pos > context.intervals[index].get_start();
				}
			);

			// insert the new element at the found position
			m_unhandled.insert(it, static_cast<i32>(new_register_index));
		}

		const u64 use_count = interval->get_use_count();

		// loop through the uses in reverse
		for(u64 i = use_count; i-- > 0;) {
			const u64 split_count = use_count - (i + 1);

			if(interval->get_use(i).get_position() > pos && split_count > 0) {
				// create a new vector for the split uses
				std::vector uses(
					interval->get_uses().begin() + i + 1, interval->get_uses().end()
				);

				// resize the original vector
				uses.resize(i + 1);

				// update it uses and interval uses
				new_interval.set_uses(uses);
				interval->set_uses(uses);
				break;
			}
		}

		// split ranges
		for(u64 i = 0; i < interval->get_ranges().size();) {
			range& interval_range = interval->get_range(i);

			if(interval_range.get_start() > pos) {
				// append the range to it ranges
				new_interval.add_range(interval_range);

				// update the interval
				interval = &context.intervals[new_register_index];

				// remove and shift
				interval->get_ranges().erase(interval->get_ranges().begin() + i);
			}
			else if(interval_range.get_end() > pos) {
				// intersects pos, we need to split the range
				range new_interval_range = { pos, interval_range.get_end() };
				interval_range.set_end(pos);

				// add the new range to it ranges
				new_interval.add_range(new_interval_range);

				// update the interval
				interval = &context.intervals[new_register_index];
				continue;
			}

			++i;
		}

		context.intervals[new_register_index] = new_interval;

		// insert move
		insert_split_move(
			context, pos, register_index, static_cast<i32>(new_register_index)
		);

		// reload before next use
		if(is_spill) {
			for(auto i = new_interval.get_uses().rbegin(); i != new_interval.get_uses().rend(); ++i) {
				if((*i).get_kind() == use_position::reg) {
					// new split
					split_intersecting(
						context, current_time, (*i).get_position() - 1,
						&context.intervals[new_register_index], false
					);

					break;
				}
			}
		}

		return static_cast<i32>(new_register_index);
	}

	live_interval* linear_scan_allocator::split_interval_at(
		code_generator_context& context, live_interval* interval, i32 pos
	) {
		// skip past previous intervals
		while(interval->get_split_kid() >= 0 && pos > interval->get_end()) {
			interval = &context.intervals[interval->get_split_kid()];
		}

		ASSERT(
			interval->get_register().get_id() >= 0 || pos <= interval->get_end(),
			"panic"
		);

		return interval;
	}

	i32 linear_scan_allocator::next_use(
		code_generator_context& context, live_interval* interval, i32 time
	) {
		while (true) {
			for(const use_position& use : interval->get_uses()) {
				if(use.get_position() > time) {
					return use.get_position();
				}
			}

			if (interval->get_split_kid() >= 0) {
				interval = &context.intervals[interval->get_split_kid()];
			}
		}
	}

	ptr_diff linear_scan_allocator::allocate_free_reg(
		code_generator_context& context, live_interval* interval
	) {
		const i32 register_class = interval->get_register().get_class();

		// callee saved will be biased to have nearer free positions to avoid
		// incurring a spill on them early
		constexpr i32 half_free = 1 << 16;
		for(i32 i = 0; i < 16; ++i) {
			m_free_positions[i] = (m_callee_saved[register_class] & (1ull << i)) ? 
				half_free :
				std::numeric_limits<i32>::max();
		}

		// for each active reg, set the free position to 0
		foreach_set(m_active_set[register_class], [&](u64 i) {
			m_free_positions[i] = 0;
		});

		// for each inactive which intersects current
		for (u64 i = 0; i < m_inactive.size(); ++i) {
			live_interval& inactive_interval = context.intervals[m_inactive[i]];
			const i32 free_position = m_free_positions[inactive_interval.get_assigned()];

			if (free_position > 0) {
				const i32 position = range_intersect(
					interval->get_start(), interval->get_end(), 
					inactive_interval.get_range(inactive_interval.get_active_range())
				);

				if (position >= 0 && position < free_position) {
					m_free_positions[inactive_interval.get_assigned()] = position;
				}
			}
		}

		// clear reserved registers
		if (register_class == static_cast<i32>(register_class::gpr)) {
			m_free_positions[static_cast<i32>(gpr::rbp)] = 0;
			m_free_positions[static_cast<i32>(gpr::rsp)] = 0;
		}

		// check if we have a valid hint
		i32 highest = -1;

		if(interval->get_hint() >= 0) {
			live_interval& hint = context.intervals[interval->get_hint()];

			ASSERT(
				hint.get_register().get_class() == register_class,
				"invalid register class"
			);

			const i32 hint_reg = hint.get_assigned();

			if(interval->get_end() <= m_free_positions[hint_reg]) {
				highest = hint_reg;
			}
		}

		// pick the highest free position
		if(highest < 0) {
			highest = 0;

			for(i32 i = 1; i < 16; ++i) {
				if(m_free_positions[i] > m_free_positions[highest]) {
					highest = i;
				}
			}
		}

		const i32 position = m_free_positions[highest];
		if(position == 0) {
			// allocation failure
			return -1;
		}

		if(m_callee_saved[register_class] & (1ull << highest)) {
			m_callee_saved[register_class] &= ~(1ull << highest);

			const u64 size = register_class ? 16 : 8;
			const i32 virtual_register = static_cast<i32>((register_class ? register_class::first_xmm : register_class::first_gpr)) + highest;
			context.stack_usage = utility::align(context.stack_usage + size, size);

			const live_interval new_interval(
				reg(-1, -1),
				context.intervals[virtual_register].get_data_type(),
				-1
			);

			const i32 register_index = interval - context.intervals.data();
			const u64 spill_slot = context.intervals.size();

			context.intervals.push_back(new_interval);

			// insert the spill and reload
			insert_split_move(
				context, 0, virtual_register, static_cast<i32>(spill_slot)
			);

			insert_split_move(
				context, context.epilogue, static_cast<i32>(spill_slot), virtual_register
			);

			// adding to intervals might resize this
			interval = &context.intervals[register_index];
		}

		if(interval->get_end() <= position) {}
		else {
			interval->set_assigned(highest);

			split_intersecting(
				context, interval->get_start(), position - 1, interval, true
			);
		}

		return highest;
	}

	ptr_diff linear_scan_allocator::allocate_blocked_reg(
		code_generator_context& context, live_interval* interval
	) {
		const i32 register_class = interval->get_register().get_class();

		for(u64 i = 0; i < 16; ++i) {
			m_block_positions[i] = std::numeric_limits<i32>::max();
			m_free_positions[i] = std::numeric_limits<i32>::max();
		}

		// mark non-fixed intervals
		foreach_set(m_active_set[register_class], [&](u64 i) {
			live_interval& it = context.intervals[m_active[register_class][i]];

			if (
				it.get_register().get_class() == register_class && 
				it.get_register().get_id() < 0
			) {
				m_free_positions[i] = next_use(
					context, &it, interval->get_start()
				);
			}
		});

		for (u64 i = 0; i < m_inactive.size(); ++i) {
			live_interval& inactive_interval = context.intervals[m_inactive[i]];

			if (
				inactive_interval.get_register().get_class() == register_class &&
				inactive_interval.get_register().get_id() < 0
			) {
				m_free_positions[i] = next_use(
					context, &inactive_interval, interval->get_start()
				);
			}
		}

		// mark fixed intervals
		foreach_set(m_active_set[register_class], [&](u64 i) {
			live_interval& fixed_intervals = context.intervals[
				m_active[register_class][i]
			];

			if (
				fixed_intervals.get_register().get_class() == register_class && 
				fixed_intervals.get_register().get_id() >= 0
			) {
				m_free_positions[i] = 0;
				m_block_positions[i] = 0;
			}
		});

		for(const i32 inactive : m_inactive) {
			live_interval& inactive_interval = context.intervals[inactive];

			if (
				inactive_interval.get_register().get_class() == register_class && 
				inactive_interval.get_register().get_id() >= 0
			) {
				const i32 block_position = m_block_positions[
					inactive_interval.get_assigned()
				];

				if (block_position > 0) {
					const i32 position = range_intersect(
						interval->get_start(), interval->get_end(),
						inactive_interval.get_range(
							inactive_interval.get_active_range()
						)
					);

					if (position >= 0 && position < block_position) {
						m_block_positions[inactive_interval.get_assigned()] = position;
					}
				}
			}
		}

		if (register_class == static_cast<i32>(register_class::gpr)) {
			m_free_positions[static_cast<i32>(gpr::rbp)] = 0;
			m_free_positions[static_cast<i32>(gpr::rsp)] = 0;
		}

		// pick highest use pos
		i32 highest = 0;
		for (int i = 1; i < 16; ++i) {
			if (m_free_positions[i] > m_free_positions[highest]) {
				highest = i;
			}
		}

		const i32 position = m_free_positions[highest];
		const i32 first_use_position = interval->get_uses().back().get_position();
		bool spilled = false;

		if (first_use_position > position) {
			constexpr u64 size = 8;

			// spill interval
			context.stack_usage = utility::align(context.stack_usage + size, size);
			interval->set_spill(static_cast<i32>(context.stack_usage));

			// split at optimal spot before first use that requires a register
			for (u64 i = interval->get_use_count(); i-- > 0;) {
				if (
					interval->get_use(i).get_position() >= position && 
					interval->get_use(i).get_kind() == use_position::reg
				) {
					split_intersecting(
						context, interval->get_start(), 
						interval->get_use(i).get_position() - 1, interval, false
					);

					break;
				}
			}

			spilled = true;
		}
		else {
			const i32 split_position = (interval->get_start() & ~1) - 1;

			// split active or inactive interval reg
			if (m_active_set[register_class].get(highest)) {
				split_intersecting(
					context, interval->get_start(), split_position, 
					&context.intervals[m_active[register_class][highest]], true
				);
			}

			// split any inactive interval for reg at the end of it's lifetime hole
			for (const i32 inactive : m_inactive) {
				live_interval* inactive_interval = &context.intervals[inactive];
				range& interval_range = inactive_interval->get_range(
					inactive_interval->get_active_range()
				);

				if (
					inactive_interval->get_register().get_class() == register_class &&
					inactive_interval->get_assigned() == highest &&
					interval_range.get_start() <= position + 1 &&
					position <= interval_range.get_end()
				) {
					split_intersecting(
						context, interval->get_start(), split_position,
						inactive_interval, true
					);
				}
			}
		}

		// split active reg if it intersects with fixed interval
		live_interval* fix_interval = &context.intervals[
			static_cast<i32>((register_class ? register_class::first_xmm : register_class::first_gpr)) + highest
		];

		if (!fix_interval->get_ranges().empty()) {
			const i32 intersect_position = range_intersect(
				interval->get_start(), interval->get_end(), 
				fix_interval->get_range(fix_interval->get_active_range())
			);

			if (intersect_position >= 0) {
				split_intersecting(
					context, interval->get_start(), intersect_position, 
					fix_interval, true
				);
			}
		}

		return spilled ? -1 : highest;
	}
}
