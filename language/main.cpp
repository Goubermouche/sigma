#include "source/codegen/codegen_visitor.h"
#include "source/parser/parser.h"

using namespace channel::types;

int main() {
	channel::parser parser("test/main.ch");
	const std::vector<channel::node*> program = parser.parse();
	channel::codegen_visitor visitor;

	for (channel::node* node : program) {
		node->accept(visitor);
	}

	// ... use the generated LLVM module for obj generation and optimization

	return 0;
}