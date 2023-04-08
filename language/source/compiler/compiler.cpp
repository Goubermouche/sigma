#include "compiler.h"
#include "../codegen/codegen_visitor.h"

namespace channel {
	void compiler::compile(const std::string& source_file) {
		std::cout << "compiling file: " << source_file << '\n';

		timer timer;
		timer.start();

		parser parser(source_file);
		codegen_visitor visitor;

		if (!visitor.generate(parser)) {
			// compilation failure
			return;
		}

		visitor.verify_intermediate_representation();
		std::cout << "[compiler]: compiled in " << timer.elapsed() << " ms\n";
		std::cout << "----------------------------\n";
		visitor.print_intermediate_representation();
	}
}