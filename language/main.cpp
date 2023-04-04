#include "source/codegen/codegen_visitor.h"
#include "source/parser/parser.h"
#include "source/utility/timer.h"

using namespace channel::types;

int main() {
	channel::timer timer;
	timer.start();

	channel::parser parser("test/main.ch");
	const std::vector<channel::node*> program = parser.parse();
	channel::codegen_visitor visitor;
	std::cout << "node count: " << program.size() << '\n';

	for (channel::node* node : program) {
		// std::cout << "**accepting top level node:\n";
		node->accept(visitor);
	}

	visitor.verify_intermediate_representation();
	visitor.print_intermediate_representation();

	std::cout << "-----------------------------\n";
	std::cout << "[channel]: took " << timer.elapsed() << "ms\n";

	return 0;
}