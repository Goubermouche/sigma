#pragma once
#include "intermediate_representation/code_generation/allocators/allocator_base.h"
#include "intermediate_representation/code_generation/live_interval.h"
#include <utility/containers/dense_set.h>

#define REGISTER_CLASS_COUNT 2
#define WIN64_ABI_CALLER_SAVED ((1u << static_cast<int>(gpr::rax)) | (1u << static_cast<int>(gpr::rcx)) | (1u << static_cast<int>(gpr::rdx)) | (1u << static_cast<int>(gpr::r8)) | (1u << static_cast<int>(gpr::r9)) | (1u << static_cast<int>(gpr::r10)) | (1u << static_cast<int>(gpr::r11)))

namespace ir::cg {
	struct parameter_descriptor {
		i32 gpr_count;
		i32 xmm_count;
		u16 caller_saved_xmm_count;
		u16 caller_saved_gpr_count;

		gpr gprs[6];
	};

	static constexpr parameter_descriptor g_parameter_descriptors[] = {
		{
			4, 4, 6, WIN64_ABI_CALLER_SAVED,
			{ gpr::rcx, gpr::rdx, gpr::r8, gpr::r9, gpr::none, gpr::none }
		}
	};

	class linear_scan_allocator : public allocator_base {
	public:
		/**
		 * \brief Attempts to allocate the used registers (defined by live ranges).
		 * In the case of allocation failure the system falls back to spilling
		 * values into the stack.
		 * \param context Code generation context
		 */
		void allocate(code_generator_context& context) override;
	private:
		void clear();

		static void reverse_block_walk(
			code_generator_context& context,
			machine_block& block,
			handle<instruction> inst
		);

		void mark_callee_saved_constraints();

		u64 partition(
			const std::vector<live_interval>& intervals,
			ptr_diff lo,
			ptr_diff hi,
			std::vector<i32>& arr
		) const;

		void quick_sort_definitions(
			std::vector<live_interval>& intervals,
			ptr_diff lo,
			ptr_diff hi,
			std::vector<i32>& arr
		);

		bool update_interval(
			const code_generator_context& context,
			live_interval* interval,
			bool is_active,
			i32 time,
			i32 inactive_index
		);

		void move_to_active(
			const code_generator_context& context,
			live_interval* interval
		);

		static i32 range_intersect(i32 start, i32 end, range& b);

		void insert_split_move(
			code_generator_context& context,
			i32 t,
			i32 old_reg,
			i32 new_reg
		);

		i32 split_intersecting(
			code_generator_context& context,
			i32 current_time,
			i32 pos,
			live_interval* interval,
			bool is_spill
		);

		static live_interval* split_interval_at(
			code_generator_context& context,
			live_interval* interval,
			i32 pos
		);

		static i32 next_use(
			code_generator_context& context,
			live_interval* interval,
			i32 time
		);

		ptr_diff allocate_free_reg(
			code_generator_context& context,
			live_interval* interval
		);

		ptr_diff allocate_blocked_reg(
			code_generator_context& context,
			live_interval* interval
		);
	private:
		utility::dense_set m_active_set[REGISTER_CLASS_COUNT];

		i32 m_active[REGISTER_CLASS_COUNT][16];
		u64 m_callee_saved[REGISTER_CLASS_COUNT];

		std::vector<i32> m_unhandled;
		std::vector<i32> m_free_positions;
		std::vector<i32> m_block_positions;
		std::vector<i32> m_inactive;

		handle<instruction> m_cache;

		i32 m_end_point;
	};
}

// TODO: remove this
#undef WIN64_ABI_CALLER_SAVED