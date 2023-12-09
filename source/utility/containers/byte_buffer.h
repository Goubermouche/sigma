#pragma once
#include "utility/containers/contiguous_container.h"
#include "utility/memory.h"

namespace utility {
	/**
	 * \brief Simple abstraction for a list of bytes. Provides basic quality of life
	 * features such as resolving relocations and iterators.
	 */
	class byte_buffer : public contiguous_container<byte> {
	public:
		byte_buffer() = default;

		byte_buffer(const contiguous_container& container)
			: contiguous_container(container) {}

		byte_buffer(const slice<byte>& slice)
			: contiguous_container(slice.get_size(), slice.get_data()) {}

		byte_buffer(u64 size)
			: contiguous_container(size) {}

		auto operator[](u64 index) const -> const byte& {
			return m_data[index];
		}

		auto operator[](u64 index) -> byte& {
			return m_data[index];
		}

		auto get_data() -> byte* {
			return m_data;
		}

		auto get_data() const -> const byte* {
			return m_data;
		}

		void append_byte(u8 data) {
			push_back(data);
		}

		void append_word(u16 data) {
			byte bytes[2];
			std::memcpy(bytes, &data, sizeof(data));
			push_back(bytes[0]);
			push_back(bytes[1]);
		}

		void append_dword(u32 data) {
			byte bytes[4];
			std::memcpy(bytes, &data, sizeof(data));
			for (int i = 0; i < 4; ++i) {
				push_back(bytes[i]);
			}
		}

		void append_qword(u64 data) {
			byte bytes[8];
			std::memcpy(bytes, &data, sizeof(data));
			for (int i = 0; i < 8; ++i) {
				push_back(bytes[i]);
			}
		}

		template<typename type>
		void append_type(const type& value) {
			// get a pointer to the memory representation of value
			const auto bytes = reinterpret_cast<const byte*>(&value);
			insert(end(), bytes, bytes + sizeof(type));
		}

		auto get_byte(u64 index) const -> u8 {
			return m_data[index];
		}

		auto get_word(u64 index) const -> u16 {
			u16 result;
			std::memcpy(&result, &m_data[index], sizeof(result));
			return result;
		}

		auto get_dword(u64 index) const -> u32 {
			u32 result;
			std::memcpy(&result, &m_data[index], sizeof(result));
			return result;
		}

		auto get_qword(u64 index) const -> u64 {
			u64 result;
			std::memcpy(&result, &m_data[index], sizeof(result));
			return result;
		}

		void patch_byte(u64 pos, u8 data) const {
			*reinterpret_cast<u8*>(&m_data[pos]) = data;
		}

		void patch_word(u64 pos, u16 data) const {
			*reinterpret_cast<u16*>(&m_data[pos]) = data;
		}

		void patch_dword(u64 pos, u32 data) const {
			*reinterpret_cast<u32*>(&m_data[pos]) = data;
		}

		void patch_qword(u64 pos, u64 data) const {
			*reinterpret_cast<u64*>(&m_data[pos]) = data;
		}

		/**
		 * \brief Resolves a chain of 32-bit relocations.
		 * \param head Pointer to the head of the relocation chain
		 * \param target Target address, to which the new relocations should point to
		 */
		void resolve_relocation_dword(u32* head, u64 target) const {
			u32 current = *head;

			// walk previous relocations
			while (current != 0 && (current & 0x80000000) == 0) {
				const u32 next = *reinterpret_cast<u32*>(&m_data[current]);
				patch_dword(current, target - (current + 4));
				current = next;
			}

			// store the target and mark it as resolved
			*head = 0x80000000 | target;
		}

		/**
		 * \brief Emits a 32-bit relocation.
		 * \param head Pointer to the head of the relocation chain
		 * \param pos Position in the buffer where the relocation should be emitted
		 */
		void emit_relocation_dword(u32* head, u64 pos) const {
			const u32 current = *head;

			if (current & 0x80000000) {
				const u32 target = current & 0x7FFFFFFF;
				patch_dword(pos, target - (pos + 4));
			}
			else {
				patch_dword(pos, current);
				*head = pos;
			}
		}
	};

	class byte_writer : public byte_buffer {
	public:
		byte_writer(u64 size) : byte_buffer(zero_initialize(size)), m_position(0) {}

		template<typename append_type>
		void write(const append_type& value) {
			std::memcpy(&m_data[m_position], &value, sizeof(append_type));
			m_position += sizeof(append_type);
		}

		[[nodiscard]] u64 get_position() const {
			return m_position;
		}
	protected:
		u64 m_position;
	};
}
