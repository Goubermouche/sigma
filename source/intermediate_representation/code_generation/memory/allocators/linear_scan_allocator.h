#pragma once
#include "intermediate_representation/code_generation/memory/allocators/allocator_base.h"
#include "intermediate_representation/code_generation/live_interval.h"
#include "intermediate_representation/code_generation/targets/x64/registers.h"

#include <utility/containers/dense_set.h>

#define REGISTER_CLASS_COUNT 2
#define WIN64_ABI_CALLER_SAVED ((1u << static_cast<int>(rax)) | (1u << static_cast<int>(rcx)) | (1u << static_cast<int>(rdx)) | (1u << static_cast<int>(r8)) | (1u << static_cast<int>(r9)) | (1u << static_cast<int>(r10)) | (1u << static_cast<int>(r11)))

namespace ir::cg {
	struct parameter_descriptor {
		i32 gpr_count;
		i32 xmm_count;
		u16 caller_saved_xmm_count;
		u16 caller_saved_gpr_count;

		i32 gprs[6];
	};

	static constexpr parameter_descriptor g_parameter_descriptors[] = {{
		4, 4, 6, WIN64_ABI_CALLER_SAVED,
		{ rcx, rdx, r8, r9, reg::invalid_id, reg::invalid_id }
	}};

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
			const std::vector<live_interval>& intervals, ptr_diff lo,
			ptr_diff hi, std::vector<u64>& arr
		) const;

		void quick_sort_definitions(
			std::vector<live_interval>& intervals, ptr_diff lo, 
			ptr_diff hi, std::vector<u64>& arr
		);

		bool update_interval(
			const code_generator_context& context, live_interval* interval,
			bool is_active, u64 time, i32 inactive_index
		);

		void move_to_active(
			const code_generator_context& context, live_interval* interval
		);

		static i32 range_intersect(u64 start, u64 end, range& b);

		void insert_split_move(
			code_generator_context& context, u64 t, ptr_diff old_reg,
			ptr_diff new_reg
		);

		i32 split_intersecting(
			code_generator_context& context, u64 current_time, u64 pos,
			live_interval* interval, bool is_spill
		);

		static live_interval* split_interval_at(
			code_generator_context& context, live_interval* interval, u64 pos
		);

		static u64 next_use(
			code_generator_context& context, live_interval* interval, u64 time
		);

		reg allocate_free_reg(
			code_generator_context& context, live_interval* interval
		);

		reg allocate_blocked_reg(
			code_generator_context& context, live_interval* interval
		);
	private:
		utility::dense_set m_active_set[REGISTER_CLASS_COUNT];

		ptr_diff m_active[REGISTER_CLASS_COUNT][16];
		u64 m_callee_saved[REGISTER_CLASS_COUNT];

		std::vector<u64> m_block_positions;
		std::vector<u64> m_free_positions;
		std::vector<u64> m_unhandled;
		std::vector<u64> m_inactive;

		handle<instruction> m_cache;
	};
}

// TODO: remove this
#undef WIN64_ABI_CALLER_SAVED