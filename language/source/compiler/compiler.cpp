#include "compiler.h"

namespace channel {
	compiler::compiler(const std::string& source_file, const std::vector<std::string>& arguments) {
		timer timer; timer.start();
		parser parser("test/main.ch");
		codegen_visitor visitor;
		const std::vector<node*> abstract_syntax_tree = parser.parse();

		for (node* node : abstract_syntax_tree) {
			node->accept(visitor);
		}

		visitor.verify_intermediate_representation();
		std::cout << "[compiler]: compiled " << abstract_syntax_tree.size() << " nodes in " << timer.elapsed() << "ms\n\n";
		visitor.print_intermediate_representation();
	}
}