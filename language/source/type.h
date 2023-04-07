#pragma once
#include <llvm/IR/IRBuilder.h>
#include "lexer/token.h"

namespace channel {
	enum class type {
		i8,
		i16,
		i32,
		i64,
		u8,
		u16,
		u32,
		u64,
	};

	static type token_to_type(token tok) {
		static const std::unordered_map<token, type> token_to_type_map = {
			{ token::keyword_type_i8 , type::i8  },
			{ token::keyword_type_i16, type::i16 },
			{ token::keyword_type_i32, type::i32 },
			{ token::keyword_type_i64, type::i64 },
			{ token::keyword_type_u8 , type::u8  },
			{ token::keyword_type_u16, type::u16 },
			{ token::keyword_type_u32, type::u32 },
			{ token::keyword_type_u64, type::u64 },
		};

		const auto it = token_to_type_map.find(tok);
		ASSERT(it != token_to_type_map.end(), "token '" + token_to_string(tok) + "' cannot be converted to a type");
		return it->second;
	}

	static llvm::Type* type_to_llvm_type(type ty, llvm::LLVMContext& context) {
		using type_function = std::function<llvm::Type* (llvm::LLVMContext&)>;
		static const std::unordered_map<type, type_function> type_to_llvm_type_map = {
			{ type::i8 ,[](llvm::LLVMContext& ctx) {return llvm::Type::getInt8Ty (ctx); } },
			{ type::i16,[](llvm::LLVMContext& ctx) {return llvm::Type::getInt16Ty(ctx); } },
			{ type::i32,[](llvm::LLVMContext& ctx) {return llvm::Type::getInt32Ty(ctx); } },
			{ type::i64,[](llvm::LLVMContext& ctx) {return llvm::Type::getInt64Ty(ctx); } },
			{ type::u8 ,[](llvm::LLVMContext& ctx) {return llvm::Type::getInt8Ty (ctx); } },
			{ type::u16,[](llvm::LLVMContext& ctx) {return llvm::Type::getInt16Ty(ctx); } },
			{ type::u32,[](llvm::LLVMContext& ctx) {return llvm::Type::getInt32Ty(ctx); } },
			{ type::u64,[](llvm::LLVMContext& ctx) {return llvm::Type::getInt64Ty(ctx); } }
		};

		const auto it = type_to_llvm_type_map.find(ty);
		return it->second(context);
	}
}
