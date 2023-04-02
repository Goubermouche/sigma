#include "source/codegen/codegen_visitor.h"
#include "source/parser/parser.h"

using namespace channel::types;

int main() {
	channel::parser parser("test/main.ch");
	const std::vector<channel::node*> program = parser.parse();
	channel::codegen_visitor visitor;
	std::cout << "node count: " << program.size() << '\n';

	for (channel::node* node : program) {
		node->accept(visitor);
	}

	visitor.print_code();
	return 0;
}