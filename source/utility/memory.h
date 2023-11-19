#pragma once
#include "utility/diagnostics/console.h"

namespace utility {
	struct byte {
		byte() = default;
 		byte(u8 value);
		[[nodiscard]] auto to_hex() const-> std::string;

		auto operator |=(const byte& other) -> byte;
		auto operator &=(const byte& other)->byte;
		auto operator <<(i32 distance) const->byte;
		auto operator >>(i32 distance) const->byte;
		auto operator +(const byte& other) const->byte;
		operator u8() const;

		u8 value;
	};

	/**
	 * \brief Aligns the specified \a value into blocks of the specified width.
	 * \param value Value to align
	 * \param alignment Alignment to use
	 * \return Aligned \a value.
	 */
	[[nodiscard]] auto align(u64 value, u64 alignment) -> u64;

	[[nodiscard]] auto sign_extend(u64 src, u64 src_bits, u64 dst_bits) -> u64;

	[[nodiscard]] auto fits_into_8_bits(u64 value) -> bool;
	[[nodiscard]] auto fits_into_32_bits(u64 value) -> bool;

	[[nodiscard]] auto pop_count(u32 value) -> u8;
}
