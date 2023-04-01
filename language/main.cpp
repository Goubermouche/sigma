#include "source/codegen/codegen_visitor.h"
#include "source/parser/parser.h"
#include <typeinfo>

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "source/codegen/abstract_syntax_tree/keywords/declaration_node.h"
#include "source/codegen/abstract_syntax_tree/keywords/types/keyword_i32_node.h"

using namespace channel::types;

int main() {
	channel::parser parser("test/main.ch");
	const std::vector<channel::node*> program = parser.parse();
	channel::codegen_visitor visitor;

	std::cout << "node count: " << program.size() << '\n';

	for (channel::node* node : program) {
		std::cout << node->get_node_name() << '\n';

		node->accept(visitor);
	}

	std::cout << "-----------------------------\n";
	visitor.m_module->print(llvm::outs(), nullptr);

	return 0;
}