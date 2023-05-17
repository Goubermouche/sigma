#pragma once
#include <llvm/IR/IRBuilder.h>
#include "lexer/token.h"

namespace channel {
	/**
	 * \brief A class for representing data types and their characteristics.
	 */
	struct type {
		/**
		 * \brief Enum representing basic types.
		 */
		enum class base {
			i8,            // 8 bit signed integer
			i16,           // 16 bit signed integer
			i32,           // 32 bit signed integer
			i64,           // 64 bit signed integer
			u8,            // 8 bit unsigned integer
			u16,           // 16 bit unsigned integer
			u32,           // 32 bit unsigned integer
			u64,           // 64 bit unsigned integer
			f32,           // 32 bit floating point (single)
			f64,           // 64 bit floating point (double)
			function,      // function
			function_call, // function call
			empty,         // void
			character,     // char
			boolean,       // bool
			unknown,
		};

		type() = default;

		/**
		 * \brief Constructor that initializes the base type and the pointer level
		 * \param base Base type
		 * \param pointer_level Level of pointer indirection
		 */
		type(
			base base, 
			u16 pointer_level
		);

		/**
		 * \brief Constructor that initializes the base type using a token and the pointer level
		 * \param tok Token to convert to a base type
		 * \param pointer_level Level of pointer indirection
		 */
		 type(
			token tok,
			u16 pointer_level
		);

		/**
		 * \brief Factory method used for creating an unknown type.
		 * \return Type with base::unknown.
		 */
		static type unknown();

		/**
		 * \brief Checks if the type is a pointer.
		 * \return True if the type is a pointer, false otherwise.
		 */
		bool is_pointer() const;

		/**
		 * \brief Checks if the type is unknown.
		 * \return True if the type is unknown, false otherwise.
		 */
		bool is_unknown() const;

		/**
		 * \brief Gets the type that this pointer type points to.
		 * \return The element type if this is a pointer type.
		 */
		type get_element_type() const;

		/**
		 * \brief Gets a type that points to the current type.
		 * \return The pointer type.
		 */
		type get_pointer_type() const;

		/**
		 * \brief Sets the pointer level for this type (levels of pointer indirection).
		 * \param pointer_level Pointer indirection level
		 */
		void set_pointer_level(
			u16 pointer_level
		);

		/**
		 * \brief Gets the pointer level for this type.
		 * \return Pointer indirection level.
		 */
		u16 get_pointer_level() const;

		/**
		 * \brief Gets the bit width of the type on the current system.
		 * \return Bit width of the current type.
		 */
		u16 get_bit_width() const;

		/**
		 * \brief Gets the base type.
		 * \return Base type of the current type.
		 */
		base get_base() const;

		/**
		 * \brief Checks if the type is signed.
		 * \return True if the type is signed, false otherwise.
		 */
		bool is_signed() const;

		/**
		 * \brief Checks if the type is unsigned.
		 * \return True if the type is unsigned, false otherwise.
		 */
		bool is_unsigned() const;

		/**
		 * \brief Checks if the type is floating point.
		 * \return True if the type is floating point, false otherwise.
		 */
		bool is_floating_point() const;

		/**
		 * \brief Checks if the type is integral.
		 * \return True if the type is integral, false otherwise.
		 */
		bool is_integral() const;

		/**
		 * \brief Checks if the type is numerical.
		 * \return True if the type is numerical, false otherwise.
		 */
		bool is_numerical() const;

		/**
		 * \brief Creates an LLVM block representation of the current type.
		 * \param context Context to use when creating the LLVM block
		 * \return LLVM type which represents the source type.
		 */
		llvm::Type* get_llvm_type(
			llvm::LLVMContext& context
		) const;

		/**
		 * \brief Generates a string representation of the source type.
		 * \return String containing the representation of the source type.
		 */
		std::string to_string() const;

		/**
		 * \brief Compares the two types.
		 * \param other Second type to compare against 
		 * \return True if both types contain the same data, false otherwise.
		 */
		bool operator==(
			const type& other
		) const;
	private:
		base m_base = base::unknown;
		u16 m_pointer_level;
	};

	/**
	 * \brief Checks if \a tok is a type token.
	 * \param tok Token to check
	 * \return True if the given token is a type token, false otherwise.
	 */
	bool is_token_type(
		token tok
	);

	/**
	 * \brief Checks which type of the two given type is higher precision.
	 * \param left_type Left type to check
	 * \param right_type Right type to check
	 * \return Higher precision type of the two types.
	 */
	type get_highest_precision_type(
		const type& left_type, 
		const type& right_type
	);
}
