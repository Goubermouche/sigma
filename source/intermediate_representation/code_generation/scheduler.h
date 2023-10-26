#pragma once
#include "intermediate_representation/code_generation/code_generator_context.h"

// Implementation of a node scheduling algorithm inspired by the Cuik TB backend. 
// Original implementation: "Global Code Motion Global Value Numbering", Cliff Click 1995

namespace ir::cg {
	/**
	 * \brief Schedules all nodes in the given \a context
	 * \param context Code generation context
	 */
	void schedule_nodes(code_generator_context& context);

	/**
	 * \brief Early scheduling phase for the given node \a n. Attempts to place the
	 * node as soon as its operands are available.
	 * \param context Code generation context
	 * \param n Node to schedule
	 */
	void schedule_early(code_generator_context& context, handle<node> n);

	/**
	 * \brief Late scheduling phase for the given node \a n. Attempts to place node as
	 * close as possible to its users without violating data dependencies. Pinned nodes
	 * remain fixed and are not rescheduled. 
	 * \param context Code generation context
	 * \param n Node to schedule
	 */
	void schedule_late(code_generator_context& context, handle<node> n);
}
