#pragma once
#include "diagnostics.h"

namespace utility {
	struct byte {
		byte() = default;
		byte(u8 value) : value(value) {}

		[[nodiscard]] auto to_hex() const -> std::string {
			std::ostringstream oss;
			oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(value);
			return oss.str();
		}

		auto operator |=(const byte& other) -> byte {
			value |= other.value;
			return *this;
		}

		auto operator &=(const byte& other) -> byte {
			value &= other.value;
			return *this;
		}

		auto operator <<(i32 distance) const -> byte {
			return byte(value << distance);
		}

		auto operator >>(i32 distance) const -> byte {
			return byte(value >> distance);
		}

		auto operator +(const byte& other) const->byte {
			return value + other.value;
		}

		operator u8() const {
			return value;
		}

		u8 value;
	};

	/**
	 * \brief Aligns the specified \a value into blocks of the specified width.
	 * \param value Value to align
	 * \param alignment Alignment to use
	 * \return Aligned \a value.
	 */
	[[nodiscard]] inline auto align(u64 value, u64 alignment) -> u64 {
		return value + (alignment - (value % alignment)) % alignment;
	}

	[[nodiscard]] inline auto sign_extend(u64 src, u64 src_bits, u64 dst_bits) -> u64 {
		const u64 sign_bit = (src >> (src_bits - 1)) & 1;

		const u64 mask_dst_bits = ~UINT64_C(0) >> (64 - dst_bits);
		const u64 mask_src_bits = ~UINT64_C(0) >> (64 - src_bits);
		const u64 mask = mask_dst_bits & ~mask_src_bits;

		// initialize dst by zeroing out the bits from src_bits to dst_bits
		const u64 dst = src & ~mask;

		// perform the actual sign extension
		return dst | (sign_bit ? mask : 0);
	}

	[[nodiscard]] auto inline fits_into_8_bits(u64 value) -> bool {
		return static_cast<u8>(value) <= std::numeric_limits<u8>::max();
	}

	[[nodiscard]] auto inline fits_into_32_bits(u64 value) -> bool {
		return static_cast<u32>(value) <= std::numeric_limits<u32>::max();
	}

	[[nodiscard]] auto inline pop_count(u32 value) -> u8 {
		return static_cast<u8>(std::bitset<32>(value).count());
	}

	[[nodiscard]] auto inline ffs(i32 value) -> u8 {
		if (value == 0) {
			return 0;
		}

		u8 pos = 1;
		while (!(value & 1)) {
			value >>= 1;
			pos++;
		}

		return pos;
	}

	[[nodiscard]] auto inline is_power_of_two(u64 value) -> bool {
		return (value & (value - 1)) == 0;
	}
} // namespace utility
