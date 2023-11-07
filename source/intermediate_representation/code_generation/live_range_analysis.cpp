#include "live_range_analysis.h"

namespace ir::cg {
	void determine_live_ranges(
		code_generator_context& context
	) {
		const u64 interval_count = context.intervals.size();
		context.epilogue = std::numeric_limits<u64>::max();

		// find block boundaries in sequences
		for(u64 i = 0; i < context.block_count; ++i) {
			context.machine_blocks[context.work_list.get_item(i)] = machine_block(interval_count);
		}

		if(context.first) {
			handle<instruction> inst = context.first;
			u64 timeline = 4;

			ASSERT(
				inst->get_type() == instruction::type::label,
				"invalid node - expected a label"
			);

			// initial label
			handle<node> bb = context.function->get_entry_node();
			handle mbb = &context.machine_blocks.at(bb);
			mbb->set_first(inst);
			mbb->set_start(2);
			inst->set_time(2);
			inst = inst->get_next_instruction();

			for(; inst; inst = inst->get_next_instruction()) {
				if(inst->get_type() == instruction::label) {
					mbb->set_end(timeline);

					ASSERT(
						inst->get_flags() & instruction::node_f,
						"label instruction has no node for the region"
					);

					bb = inst->get<node_prop>()->value;

					mbb = &context.machine_blocks.at(bb);
					mbb->set_first(inst->get_next_instruction());
					timeline += 2;
					mbb->set_start(timeline);
				}
				else if(inst->is_terminator() && mbb->get_terminator() == 0) {
					mbb->set_terminator(timeline);
				}
				else if(inst->get_type() == instruction::epilogue) {
					context.epilogue = timeline;
				}

				inst->set_time(timeline);
				timeline += 2;

				utility::dense_set& gen = mbb->get_gen();
				utility::dense_set& kill = mbb->get_kill();

				const auto inputs = inst->get_operands().begin() + inst->get_out_count();
				for (u8 i = 0; i < inst->get_in_count(); ++i) {
					if (!kill.get(inputs[i])) {
						gen.put(inputs[i]);
					}
				}

				const auto outputs = inst->get_operands().begin();
				for (u8 i = 0; i < inst->get_out_count(); ++i) {
					kill.put(outputs[i]);
				}
			}

			mbb->set_end(timeline);
		}

		// generate global live sets
		ASSERT(
			context.work_list.get_item_count() == context.block_count, 
			"invalid block count"
		);

		std::vector<handle<node>> blocks(context.block_count);

		// all nodes go into the work list
		for(u64 i = 0; i < context.block_count; ++i) {
			handle<node> block = context.work_list.get_item(i);

			ASSERT(
				block->get_type() == node::entry || block->get_type() == node::region,
				"node is not a block"
			);

			machine_block& machine_block = context.machine_blocks[block];
			machine_block.set_live_in(machine_block.get_gen());
			blocks[i] = block;
		}

		// iterate over all blocks and determine their live ranges
		while (!blocks.empty()) {
			handle<node> block = blocks.back();
			const handle<region_property> region = block->get<region_property>();
			machine_block& mbb = context.machine_blocks[block];
			utility::dense_set& live_out = mbb.get_live_out();

			blocks.pop_back();
			live_out.clear();

			// walk all successors
			if(region->end->get_type() == node::branch) {
				const handle<branch_property> branch = region->end->get<branch_property>();

				for(handle<node> successor : branch->successors) {
					machine_block& successor_block = context.machine_blocks[successor];
					live_out.set_union(successor_block.get_live_in());
				}
			}

			utility::dense_set& live_in = mbb.get_live_in();
			utility::dense_set& kill    = mbb.get_kill();
			utility::dense_set& gen     = mbb.get_gen();
			bool changes = false;

			for(u64 i = 0; i < (interval_count + 63) / 64; ++i) {
				const u64 new_in = (live_out.data(i) & ~kill.data(i)) | gen.data(i);
				changes |= (live_in.data(i) != new_in);
				live_in.data(i) = new_in;
			}

			// if we have changes, mark the predecessors
			if(changes) {
				blocks.append_range(block->get_inputs());
			}
		}

		// context.work_list.get_items().resize(context.block_count);
		ASSERT(context.epilogue != std::numeric_limits<u64>::max(), "invalid epilogue");
	}
}

