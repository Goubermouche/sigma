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

	type get_pointer_type(type ty) {
		static const std::unordered_map<type, type> type_to_type_map = {
			{ type::i8       , type::i8_pointer      },
			{ type::i16      , type::i16_pointer     },
			{ type::i32      , type::i32_pointer     },
			{ type::i64      , type::i64_pointer     },
			{ type::u8       , type::u8_pointer      },
			{ type::u16      , type::u16_pointer     },
			{ type::u32      , type::u32_pointer     },
			{ type::u64      , type::u64_pointer     },
			{ type::f32      , type::f32_pointer     },
			{ type::f64      , type::f64_pointer     },
			{ type::void_type, type::void_pointer    },
		};

		const auto it = type_to_type_map.find(ty);
		ASSERT(it != type_to_type_map.end(), "pointer type for type '" + type_to_string(ty) + "' does not exist");
		return it->second;
	}

	type get_inherent_pointer_type(type ty) {
		static const std::unordered_map<type, type> type_to_type_map = {
			{ type::i8_pointer  , type::i8        },
			{ type::i16_pointer , type::i16       },
			{ type::i32_pointer , type::i32       },
			{ type::i64_pointer , type::i64       },
			{ type::u8_pointer  , type::u8        },
			{ type::u16_pointer , type::u16       },
			{ type::u32_pointer , type::u32       },
			{ type::u64_pointer , type::u64       },
			{ type::f32_pointer , type::f32       },
			{ type::f64_pointer , type::f64       },
			{ type::void_pointer, type::void_type },
		};

		const auto it = type_to_type_map.find(ty);
		ASSERT(it != type_to_type_map.end(), "type for pointer type '" + type_to_string(ty) + "' does not exist");
		return it->second;
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

	bool is_type_pointer(type ty) {
		switch (ty) {
		case type::i8_pointer:
		case type::i16_pointer:
		case type::i32_pointer:
		case type::i64_pointer:
		case type::u8_pointer:
		case type::u16_pointer:
		case type::u32_pointer:
		case type::u64_pointer:
		case type::f32_pointer:
		case type::f64_pointer:
		case type::void_pointer:
			return true;
		default:
			return false;
		}
	}

	type token_to_type(token tok) {
		static const std::unordered_map<token, type> token_to_type_map = {
			{ token::keyword_type_i8  , type::i8        },
			{ token::keyword_type_i16 , type::i16       },
			{ token::number_signed    , type::i32       },
			{ token::keyword_type_i32 , type::i32       },
			{ token::keyword_type_i64 , type::i64       },
			{ token::keyword_type_u8  , type::u8        },
			{ token::keyword_type_u16 , type::u16       },
			{ token::number_unsigned  , type::u32       },
			{ token::keyword_type_u32 , type::u32       },
			{ token::keyword_type_u64 , type::u64       },
			{ token::number_f32       , type::f32       },
			{ token::keyword_type_f32 , type::f32       },
			{ token::number_f64       , type::f64       },
			{ token::keyword_type_f64 , type::f64       },
			{ token::keyword_type_void, type::void_type },
			{ token::keyword_type_char, type::char_type },
			// { token::pointer, type::pointer },
		};

		const auto it = token_to_type_map.find(tok);
		ASSERT(it != token_to_type_map.end(), "token '" + token_to_string(tok) + "' cannot be converted to a type");
		return it->second;
	}

	bool is_token_type(token tok) {
		static const std::unordered_map<token, type> token_type_map = {
			{ token::keyword_type_i8  , type::i8        },
			{ token::keyword_type_i16 , type::i16       },
			{ token::keyword_type_i32 , type::i32       },
			{ token::keyword_type_i64 , type::i64       },
			{ token::keyword_type_u8  , type::u8        },
			{ token::keyword_type_u16 , type::u16       },
			{ token::keyword_type_u32 , type::u32       },
			{ token::keyword_type_u64 , type::u64       },
			{ token::keyword_type_f32 , type::f32       },
			{ token::keyword_type_f64 , type::f64       },
			{ token::keyword_type_void, type::void_type },
			{ token::keyword_type_char, type::char_type },
			// { token::pointer, type::pointer },
		};

		const auto it = token_type_map.find(tok);
		return it != token_type_map.end();
	}

	std::string type_to_string(type ty)	{
		static const std::unordered_map<type, std::string> type_to_string_map = {
			{ type::i8           , "i8"            },
			{ type::i16          , "i16"           },
			{ type::i32          , "i32"           },
			{ type::i64          , "i64"           },
			{ type::u8           , "u8"            },
			{ type::u16          , "u16"           },
			{ type::u32          , "u32"           },
			{ type::u64          , "u64"           },
			{ type::f32          , "f32"           },
			{ type::f64          , "f64"           },
			{ type::function     , "function"      },
			{ type::function_call, "function_call" },
			{ type::void_type    , "void"          },
			{ type::char_type    , "char"          },
			// pointers			 
			{ type::i8_pointer   , "i8_pointer"    },
			{ type::i16_pointer  , "i16_pointer"   },
			{ type::i32_pointer  , "i32_pointer"   },
			{ type::i64_pointer  , "i64_pointer"   },
			{ type::u8_pointer   , "u8_pointer"    },
			{ type::u16_pointer  , "u16_pointer"   },
			{ type::u32_pointer  , "u32_pointer"   },
			{ type::u64_pointer  , "u64_pointer"   },
			{ type::f32_pointer  , "f32_pointer"   },
			{ type::f64_pointer  , "f64_pointer"   },
			{ type::void_pointer , "void_pointer"  },
		};

		const auto it = type_to_string_map.find(ty);
		ASSERT(it != type_to_string_map.end(), "type '" << static_cast<i32>(ty) + "' cannot be converted to a string");
		return it->second;
	}

	llvm::Type* type_to_llvm_type(type ty, llvm::LLVMContext& context) {
		using type_function = std::function<llvm::Type* (llvm::LLVMContext&)>;
		static const std::unordered_map<type, type_function> type_to_llvm_type_map = {			 
			{ type::i8          , [](llvm::LLVMContext& ctx) { return llvm::Type::getInt8Ty(ctx) ;     } },
			{ type::i16         , [](llvm::LLVMContext& ctx) { return llvm::Type::getInt16Ty(ctx);     } },
			{ type::i32         , [](llvm::LLVMContext& ctx) { return llvm::Type::getInt32Ty(ctx);     } },
			{ type::i64         , [](llvm::LLVMContext& ctx) { return llvm::Type::getInt64Ty(ctx);     } },
			{ type::u8          , [](llvm::LLVMContext& ctx) { return llvm::Type::getInt8Ty(ctx);      } },
			{ type::u16         , [](llvm::LLVMContext& ctx) { return llvm::Type::getInt16Ty(ctx);     } },
			{ type::u32         , [](llvm::LLVMContext& ctx) { return llvm::Type::getInt32Ty(ctx);     } },
			{ type::u64         , [](llvm::LLVMContext& ctx) { return llvm::Type::getInt64Ty(ctx);     } },
			{ type::f32         , [](llvm::LLVMContext& ctx) { return llvm::Type::getFloatTy(ctx);     } },
			{ type::f64         , [](llvm::LLVMContext& ctx) { return llvm::Type::getDoubleTy(ctx);    } },
			{ type::void_type   , [](llvm::LLVMContext& ctx) { return llvm::Type::getVoidTy(ctx);      } },
			{ type::char_type   , [](llvm::LLVMContext& ctx) { return llvm::Type::getInt8Ty(ctx);      } },
			// pointers			  
			{ type::i8_pointer  , [](llvm::LLVMContext& ctx) { return llvm::Type::getInt8PtrTy(ctx);   } },
			{ type::i16_pointer , [](llvm::LLVMContext& ctx) { return llvm::Type::getInt16PtrTy(ctx);  } },
			{ type::i32_pointer , [](llvm::LLVMContext& ctx) { return llvm::Type::getInt32PtrTy(ctx);  } },
			{ type::i64_pointer , [](llvm::LLVMContext& ctx) { return llvm::Type::getInt64PtrTy(ctx);  } },
			{ type::u8_pointer  , [](llvm::LLVMContext& ctx) { return llvm::Type::getInt8PtrTy(ctx);   } },
			{ type::u16_pointer , [](llvm::LLVMContext& ctx) { return llvm::Type::getInt16PtrTy(ctx);  } },
			{ type::u32_pointer , [](llvm::LLVMContext& ctx) { return llvm::Type::getInt32PtrTy(ctx);  } },
			{ type::u64_pointer , [](llvm::LLVMContext& ctx) { return llvm::Type::getInt64PtrTy(ctx);  } },
			{ type::f32_pointer , [](llvm::LLVMContext& ctx) { return llvm::Type::getFloatPtrTy(ctx);  } },
			{ type::f64_pointer , [](llvm::LLVMContext& ctx) { return llvm::Type::getDoublePtrTy(ctx); } },
			{ type::void_pointer, [](llvm::LLVMContext& ctx) { return llvm::Type::getInt8PtrTy(ctx);   } },
		};																						 

		const auto it = type_to_llvm_type_map.find(ty);
		ASSERT(it != type_to_llvm_type_map.end(), "type '" << type_to_string(ty) + "' cannot be converted to an llvm::Type*");
		return it->second(context);
	}
}