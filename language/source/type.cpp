#include "type.h"

namespace channel {
	type get_highest_precision_type(type left_type, type right_type) {
		assert(left_type != type::function && left_type != type::function_call &&
			right_type != type::function && right_type != type::function_call &&
			"Invalid types for get_highest_precision_type");

		if (is_type_floating_point(left_type) || is_type_floating_point(right_type)) {
			return (left_type == type::f64 || right_type == type::f64) ? type::f64 : type::f32;
		}

		static std::unordered_map<type, int> type_priority = {
			{ type::i8 , 1}, {type::u8 , 1 },
			{ type::i16, 2}, {type::u16, 2 },
			{ type::i32, 3}, {type::u32, 3 },
			{ type::i64, 4}, {type::u64, 4 }
		};

		return type_priority[left_type] > type_priority[right_type] ? left_type : right_type;
	}

	i32 get_type_bit_width(type ty) {
		static const std::unordered_map<type, i32> token_to_bit_width_map = {
			{ type::i8 , 8  },
			{ type::i16, 16 },
			{ type::i32, 32 },
			{ type::i64, 64 },
			{ type::u8 , 8  },
			{ type::u16, 16 },
			{ type::u32, 32 },
			{ type::u64, 64 },
			{ type::f32, 32 },
			{ type::f64, 64 },
		};

		const auto it = token_to_bit_width_map.find(ty);
		ASSERT(it != token_to_bit_width_map.end(), "invalid bit width calculation for type '" + type_to_string(ty) + "'");
		return it->second;
	}

	bool is_type_signed(type ty) {
		switch (ty) {
		case type::i8:
		case type::i16:
		case type::i32:
		case type::i64:
		case type::f32:
		case type::f64:
			return true;
		default:
			return false;
		}
	}

	bool is_type_unsigned(type ty) {
		switch (ty) {
		case type::u8:
		case type::u16:
		case type::u32:
		case type::u64:
			return true;
		default:
			return false;
		}
	}

	bool is_type_integral(type ty) {
		switch (ty) {
		case type::i8:
		case type::i16:
		case type::i32:
		case type::i64:
		case type::u8:
		case type::u16:
		case type::u32:
		case type::u64:
			return true;
		default:
			return false;
		}
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
			{ token::keyword_type_f32, type::f32 },
			{ token::keyword_type_f64, type::f64 },
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
			{ type::f32,           "f32"           },
			{ type::f64,           "f64"           },
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
			{ type::i8 ,[](llvm::LLVMContext& ctx) { return llvm::Type::getInt8Ty(ctx) ;  } },
			{ type::i16,[](llvm::LLVMContext& ctx) { return llvm::Type::getInt16Ty(ctx);  } },
			{ type::i32,[](llvm::LLVMContext& ctx) { return llvm::Type::getInt32Ty(ctx);  } },
			{ type::i64,[](llvm::LLVMContext& ctx) { return llvm::Type::getInt64Ty(ctx);  } },
			{ type::u8 ,[](llvm::LLVMContext& ctx) { return llvm::Type::getInt8Ty(ctx) ;  } },
			{ type::u16,[](llvm::LLVMContext& ctx) { return llvm::Type::getInt16Ty(ctx);  } },
			{ type::u32,[](llvm::LLVMContext& ctx) { return llvm::Type::getInt32Ty(ctx);  } },
			{ type::u64,[](llvm::LLVMContext& ctx) { return llvm::Type::getInt64Ty(ctx);  } },
			{ type::f32,[](llvm::LLVMContext& ctx) { return llvm::Type::getFloatTy(ctx);  } },
			{ type::f64,[](llvm::LLVMContext& ctx) { return llvm::Type::getDoubleTy(ctx); } },
		};

		const auto it = type_to_llvm_type_map.find(ty);
		return it->second(context);
	}
}