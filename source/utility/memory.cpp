#include "memory.h"

namespace utility {
    byte::byte(u8 value)
	    : value(value) {}

    auto byte::to_hex() const -> std::string {
        std::ostringstream oss;
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(value);
        return oss.str();
    }

    auto byte::operator|=(const byte& other) -> byte {
        value |= other.value;
        return *this;
    }

    auto byte::operator&=(const byte& other) -> byte {
        value &= other.value;
        return *this;
    }

    auto byte::operator<<(i32 distance) const -> byte {
        return byte(value << distance);
    }

    auto byte::operator>>(i32 distance) const -> byte {
        return byte(value >> distance);
    }

    auto byte::operator+(const byte& other) const -> byte {
        return value + other.value;
    }

    byte::operator u8() const {
        return value;
    }

    auto align(u64 value, u64 alignment) -> u64 {
        return (value + alignment - 1) & ~(alignment - 1);
    }

    auto sign_extend(u64 src, u64 src_bits, u64 dst_bits) -> u64 {
        const u64 sign_bit = (src >> (src_bits - 1)) & 1;

        const u64 mask_dst_bits = ~UINT64_C(0) >> (64 - dst_bits);
        const u64 mask_src_bits = ~UINT64_C(0) >> (64 - src_bits);
        const u64 mask = mask_dst_bits & ~mask_src_bits;

        // initialize dst by zeroing out the bits from src_bits to dst_bits
        const u64 dst = src & ~mask;

        // perform the actual sign extension
        return dst | (sign_bit ? mask : 0);
    }

    auto fits_into_8_bits(u64 value) -> bool {
        return static_cast<u8>(value) <= std::numeric_limits<u8>::max();
    }

    auto fits_into_32_bits(u64 value) -> bool {
        return static_cast<u32>(value) <= std::numeric_limits<u32>::max();
    }

    auto pop_count(u32 value) -> u8 {
        return static_cast<u8>(std::bitset<32>(value).count());
    }
}