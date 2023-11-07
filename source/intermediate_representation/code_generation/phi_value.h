#pragma once
#include "intermediate_representation/code_generation/live_interval.h"

// Root phi value file. Phi nodes are crucial for handling variables that have 
// multiple definitions due to control flow (ie. conditional branches). Our phi
// node collects different possible values arriving from different predecessors
// into a single phi value.

// Since we are in SSA form, we only allow a single assignment per value, say we 
// have the following if-else statement: 
// 
// if(condition) {
//     x = 1;
// }
// else {
//     x = 2;
// }
// 
// In the case above we can't know which value should be assigned before executing,
// and because of this we can use a phi node, which will decide for us. A phi node
// basically selects a value depending on the control flow of the program. When 
// control flow merges at a partical point in the program, the phi function will
// "choose" the value of one of its arguments based on the path taken to reach
// that point. If we were to rewrite the above example in pseudo-IR it would look
// something like this: 
// 
// if(condition) {
//     x1 = 1;
// }
// else {
//     x2 = 2;
// }
// 
// x3 = phi(x1, x2); ... x3 will be either x1 or x2 depending on the path

namespace ir::cg {
	class phi_value {
	public:
		phi_value(
			handle<node> n,
			handle<node> phi,
			reg source = reg(),
			reg destination = reg()
		);

		void set_destination(reg destination);
		void set_source(reg source);

		void set_node(handle<node> node);
		void set_phi(handle<node> phi);

		handle<node> get_phi() const;
		handle<node> get_node() const;
		reg get_destination() const;
	private:
		handle<node> m_node;
		handle<node> m_phi;

		reg m_destination;
		reg m_source;
	};
}
