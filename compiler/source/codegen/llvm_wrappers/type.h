#pragma once
#include <llvm/IR/IRBuilder.h>
#include "lexer/token.h"

namespace channel {
	struct type {
		enum class base {
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
			empty,         // void
			character,     // char
			boolean,       // bool
			unknown,
		};

		type() = default;
		type(base base, u16 pointer_level);
		type(token tok, u16 pointer_level);

		static type unknown();
		   
		bool is_pointer() const;
		bool is_unknown() const;

		type get_element_type() const;
		type get_pointer_type() const;

		void set_pointer_level(u16 pointer_level);
		u16 get_pointer_level() const;

		u16 get_bit_width() const;
		base get_base() const;

		bool is_signed() const;
		bool is_unsigned() const;
		bool is_floating_point() const;
		bool is_integral() const;
		bool is_numerical() const;

		llvm::Type* get_llvm_type(llvm::LLVMContext& context) const;
		std::string to_string() const;

		bool operator==(const type& other) const;
	private:
		base m_base = base::unknown;
		u16 m_pointer_level;
	};

	bool is_token_type(token tok);

	type get_highest_precision_type(const type& left_type, const type& right_type);
}
