#include "compiler.h"

namespace channel {
	compiler::compiler(const std::string& source_file, const std::vector<std::string>& arguments) {
		timer timer; timer.start();
		parser parser("test/main.ch");
		const codegen_visitor visitor(parser);

		visitor.verify_intermediate_representation();
		std::cout << "[compiler]: compiled in " << timer.elapsed() << " ms\n";
		std::cout << "----------------------------\n";
		visitor.print_intermediate_representation();
	}
}