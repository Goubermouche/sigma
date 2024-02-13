#pragma once
#include <utility/containers/dense_set.h>

#include "intermediate_representation/codegen/memory/allocators/allocator_base.h"
#include "intermediate_representation/codegen/live_interval.h"
#include "intermediate_representation/target/arch/x64/x64.h"

#define REGISTER_CLASS_COUNT 2

namespace sigma::ir {
	class linear_scan_allocator : public allocator_base {
	public:
		/**
		 * \brief Attempts to allocate the used registers (defined by live ranges).
		 * In the case of allocation failure the system falls back to spilling
		 * values into the stack.
		 * \param context Code generation context
		 */
		void allocate(codegen_context& context) override;
	private:
		void clear();

		static void reverse_block_walk(
			codegen_context& context, handle<machine_block> block, handle<instruction> inst
		);

		void mark_callee_saved_constraints(const codegen_context& context);

		static auto partition(
			const std::vector<live_interval>& intervals, 
			ptr_diff lo,
			ptr_diff hi,
			std::vector<u64>& arr
		) -> u64;

		static void quick_sort_definitions(
			std::vector<live_interval>& intervals, 
			ptr_diff lo,
			ptr_diff hi,
			std::vector<u64>& arr
		);

		auto update_interval(
			const codegen_context& context, 
			handle<live_interval> interval,
			bool is_active,
			u64 time,
			ptr_diff inactive_index
		) -> bool;

		void move_to_active(const codegen_context& context, handle<live_interval> interval);
		void insert_split_move(codegen_context& context, u64 t, ptr_diff old_reg, ptr_diff new_reg);

		auto split_intersecting(
			codegen_context& context,
			u64 current_time,
			u64 pos,
			handle<live_interval> interval, 
			bool is_spill
		) -> u64;

		auto allocate_free_reg(codegen_context& context, handle<live_interval> interval) -> reg;
		auto allocate_blocked_reg(codegen_context& context, handle<live_interval> interval) -> reg;
	private:
		utility::dense_set m_active_set[REGISTER_CLASS_COUNT] = {};

		ptr_diff m_active[REGISTER_CLASS_COUNT][16] = {};
		u32 m_callee_saved[REGISTER_CLASS_COUNT] = {};

		std::vector<ptr_diff> m_free_positions;
		std::vector<ptr_diff> m_inactive;
		std::vector<u64> m_unhandled;

		handle<instruction> m_cache;
	};
} // namespace sigma::ir
