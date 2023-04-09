#include "compiler.h"
#include "../codegen/visitor/codegen_visitor.h"

namespace channel {
	void compiler::compile(const std::string& source_file) {
		std::cout << "compiling file '" << source_file << "'\n";

		timer timer;
		timer.start();

		parser parser(source_file);
		codegen_visitor visitor;

		if (!visitor.generate(parser)) {
			// compilation failure
			return;
		}

		if(!visitor.verify_intermediate_representation()) {
			// verification failure
			return;
		}

		std::cout << "successfully compiled in " << timer.elapsed() << " ms\n";
		// std::cout << "----------------------------\n";
	}
}