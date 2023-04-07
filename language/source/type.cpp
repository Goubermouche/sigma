#include "type.h"

namespace channel {
	bool is_type_signed(type ty) {
		return ty == type::i8 ||
			ty == type::i16 ||
			ty == type::i32 ||
			ty == type::i64;
	}

	bool is_type_unsigned(type ty) {
		return ty == type::u8 ||
			ty == type::u16 ||
			ty == type::u32 ||
			ty == type::u64;
	}

	bool is_type_floating_point(type ty)	{
		return ty == type::f32 || ty == type::f64;
	}

	type token_to_type(token tok) {
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

	std::string type_to_string(type ty)	{
		static const std::unordered_map<type, std::string> type_to_string_map = {
			{ type::i8,            "i8"            },
			{ type::i16,           "i16"           },
			{ type::i32,           "i32"           },
			{ type::i64,           "i64"           },
			{ type::u8,            "u8"            },
			{ type::u16,           "u16"           },
			{ type::u32,           "u32"           },
			{ type::u64,           "u64"           },
			{ type::function,      "function"      },
			{ type::function_call, "function_call" },
		};

		const auto it = type_to_string_map.find(ty);
		ASSERT(it != type_to_string_map.end(), "type '" << static_cast<i32>(ty) + "' cannot be converted to a string");
		return it->second;
	}

	llvm::Type* type_to_llvm_type(type ty, llvm::LLVMContext& context) {
		using type_function = std::function<llvm::Type* (llvm::LLVMContext&)>;
		static const std::unordered_map<type, type_function> type_to_llvm_type_map = {
			{ type::i8 ,[](llvm::LLVMContext& ctx) { return llvm::Type::getInt8Ty(ctx); } },
			{ type::i16,[](llvm::LLVMContext& ctx) { return llvm::Type::getInt16Ty(ctx); } },
			{ type::i32,[](llvm::LLVMContext& ctx) { return llvm::Type::getInt32Ty(ctx); } },
			{ type::i64,[](llvm::LLVMContext& ctx) { return llvm::Type::getInt64Ty(ctx); } },
			{ type::u8 ,[](llvm::LLVMContext& ctx) { return llvm::Type::getInt8Ty(ctx); } },
			{ type::u16,[](llvm::LLVMContext& ctx) { return llvm::Type::getInt16Ty(ctx); } },
			{ type::u32,[](llvm::LLVMContext& ctx) { return llvm::Type::getInt32Ty(ctx); } },
			{ type::u64,[](llvm::LLVMContext& ctx) { return llvm::Type::getInt64Ty(ctx); } }
		};

		const auto it = type_to_llvm_type_map.find(ty);
		return it->second(context);
	}
}