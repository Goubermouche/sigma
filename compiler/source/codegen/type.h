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

	/**
	 * \brief Returns the highest precision type of the two specified types. 
	 * \param left_type First type
	 * \param right_type  Second type
	 * \return Type with the highest precision
	 */
	type get_highest_precision_type(type left_type, type right_type);

	/**
	 * \brief Gets the pointer type of the specified pointer.
	 * \param ty Type to get the pointer type of
	 * \return Pointer type of the specified type
	 */
	type get_pointer_type(type ty);

	/**
	 * \brief Gets the bit width of the specified type.
	 * \param ty Type to get the bit width of
	 * \return Bit width of the specified type
	 */
	i32 get_type_bit_width(type ty);

	/**
	 * \brief Checks if the specified type is signed.
	 * \param ty Type to check 
	 * \return True if the specified type is signed
	 */
	bool is_type_signed(type ty);

	/**
	 * \brief Checks if the specified type is unsigned.
	 * \param ty Type to check
	 * \return True if the specified type is unsigned
	 */
	bool is_type_unsigned(type ty);

	/**
	 * \brief Checks if the specified type is an integer.
	 * \param ty Type to check
	 * \return True if the specified type is an integer
	 */
	bool is_type_integral(type ty);

	/**
	 * \brief Checks if the specified type is a floating-point numerical type.
	 * \param ty Type to check
	 * \return True if the specified type is floating-point
	 */
	 bool is_type_floating_point(type ty);

	 /**
	  * \brief Checks if the specified type is a pointer.
	  * \param ty Type to check
	  * \return True if the specified type is a pointer type.
	  */
	 bool is_type_pointer(type ty);

	 /**
	  * \brief Converts the specified token to a type.
	  * \param tok Token to convert 
	  * \return Type of the specified token
	  */
	 type token_to_type(token tok);

	 /**
	  * \brief Checks if a token is also a type.
	  * \param tok Token to check
	  * \return True if the specified token is also a type
	  */
	 bool is_token_type(token tok);

	 /**
	  * \brief Converts the specified type to a string.
	  * \param ty Type to convert
	  * \return String representing the given type
	  */
	 std::string type_to_string(type ty);

	 /**
	  * \brief Converts the specified type into an llvm::Type.
	  * \param ty Type to convert
	  * \param context Context to use for type creation
	  * \return llvm::Type* of the specified type
	  */
	 llvm::Type* type_to_llvm_type(type ty, llvm::LLVMContext& context);
}
