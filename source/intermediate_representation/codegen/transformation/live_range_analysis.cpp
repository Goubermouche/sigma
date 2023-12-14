#include "live_range_analysis.h"

namespace sigma::ir {
	void determine_live_ranges(codegen_context& context) {
		const u64 interval_count = context.intervals.size();
		u64 epilogue = std::numeric_limits<u64>::max();

		// find block boundaries in sequences
		context.machine_blocks.reserve(context.basic_block_order.size());

		for(const u64 block_order : context.basic_block_order) {
			auto target = context.work->items[block_order];
			const auto basic_block = &context.graph.blocks.at(target);

			const machine_block machine_block{
				.end_node = basic_block->end,
				.gen = utility::dense_set(interval_count),
				.kill = utility::dense_set(interval_count),
				.live_in = utility::dense_set(interval_count),
				.live_out = utility::dense_set(interval_count)
			};

			context.machine_blocks[target] = machine_block;
		}
	
		if (context.first) {
			handle<instruction> inst = context.first;

			ASSERT(
				inst->type == instruction::LABEL,
				"entry instruction is not a label"
			);

			// initial label
			auto basic_block = context.work->items.front();
			auto machine_block = &context.machine_blocks.at(basic_block);
			u64 timeline = 4;

			machine_block->first = inst;
			machine_block->start = 2;
			inst->time = 2;
			inst = inst->next_instruction;

			for (; inst; inst = inst->next_instruction) {
				if (inst->type == instruction::LABEL) {
					context.machine_blocks.at(basic_block).end = timeline;
					timeline += 4;

					ASSERT(
						inst->flags & instruction::node_f,
						"instruction does not contain a node"
					);

					basic_block = inst->get<handle<node>>();
					machine_block = &context.machine_blocks.at(basic_block);
					machine_block->first = inst->next_instruction;
					machine_block->start = timeline;
				}
				else if (inst->is_terminator() && machine_block->terminator == 0) {
					machine_block->terminator = timeline;
				}
				else if (inst->type == instruction::EPILOGUE) {
					epilogue = timeline;
				}

				auto& gen = machine_block->gen;
				auto& kill = machine_block->kill;

				inst->time = timeline;
				timeline += 2;

				const auto inputs = inst->operands.begin() + inst->out_count;
				const auto outputs = inst->operands.begin();

				for (u8 i = 0; i < inst->in_count; ++i) {
					if (!kill.get(inputs[i])) {
						gen.put(inputs[i]);
					}
				}

				for (u8 i = 0; i < inst->out_count; ++i) {
					kill.put(outputs[i]);
				}
			}

			machine_block->end = timeline;
		}

		const u64 item_base = context.work->items.size();

		// add all our nodes into the work list
		for (const u64 block_order : context.basic_block_order) {
			auto target = context.work->items[block_order];
			context.work->items.push_back(target);

			const auto machine_block = &context.machine_blocks.at(target);
			machine_block->live_in.copy(machine_block->gen);
		}

		// generate global live sets
		while (context.work->items.size() > item_base) {
			handle<node> basic_block = context.work->items.back();
			context.work->items.pop_back();

			const auto machine_block = &context.machine_blocks.at(basic_block);
			const auto block_end = machine_block->end_node;
			auto& live_out = machine_block->live_out;

			live_out.clear();

			// walk all successors
			if (block_end->ty == node::BRANCH) {
				for (handle<user> user = block_end->use; user; user = user->next_user) {
					if (user->n->ty == node::PROJECTION) {
						// union with successor's lives
						handle<node> successor = user->n->get_next_block();
						live_out.set_union(context.machine_blocks.at(successor).live_in);
					}
				}
			}
			else if (
				block_end->ty != node::EXIT && 
				block_end->ty != node::UNREACHABLE
			) {
				// union with successor's lives
				handle<node> successor = block_end->get_next_control();
				live_out.set_union(context.machine_blocks.at(successor).live_in);
			}

			auto& live_in = machine_block->live_in;
			auto& kill = machine_block->kill;
			auto& gen = machine_block->gen;
			bool changes = false;

			for (u64 i = 0; i < (interval_count + 63) / 64; ++i) {
				const u64 new_in = (live_out.data(i) & ~kill.data(i)) | gen.data(i);
				changes |= (live_in.data(i) != new_in);

				live_in.data(i) = new_in;
			}

			// if we have changes, mark the predecessors
			if (
				changes &&
				!(basic_block->ty == node::PROJECTION && basic_block->inputs[0]->ty == node::ENTRY)
			) {
				for (u64 i = 0; i < basic_block->inputs.get_size(); ++i) {
					context.work->items.push_back(context.graph.get_predecessor(basic_block, i));
				}
			}
		}

		context.endpoint = epilogue;
	}
}
