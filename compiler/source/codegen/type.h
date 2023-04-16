#pragma once
#include <llvm/IR/IRBuilder.h>
#include "../lexer/token.h"

namespace channel {
	enum class type {
		unknown = -1,
		i8,
		i16,
		i32,
		i64,
		u8,
		u16,
		u32,
		u64,
		f32,
		f64,
		function,
		function_call,
		void_type,
		// pointers
		i8_pointer,
		i16_pointer,
		i32_pointer,
		i64_pointer,
		u8_pointer,
		u16_pointer,
		u32_pointer,
		u64_pointer,
		f32_pointer,
		f64_pointer,
		void_pointer
	};

	type get_highest_precision_type(type left_type, type right_type);
	type get_pointer_type(type ty);
	i32 get_type_bit_width(type ty);

	bool is_type_signed(type ty);
	bool is_type_unsigned(type ty);
	bool is_type_integral(type ty);
	bool is_type_floating_point(type ty);
	bool is_type_pointer(type ty);

	type token_to_type(token tok);
	bool is_token_type(token tok);

	std::string type_to_string(type ty);
	llvm::Type* type_to_llvm_type(type ty, llvm::LLVMContext& context);
}
