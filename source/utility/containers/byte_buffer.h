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
		
		/**
		 * \brief Appends a byte of data to the buffer.
		 * \param data Data to append
		 */
		void append_byte(u8 data) {
			// ASSERT(data != 0x06, "x");
			push_back(data);
		}

		/**
		 * \brief Appends 2 bytes of data to the buffer.
		 * \param data Data to append
		 */
		void append_word(u16 data) {
			byte bytes[2];
			std::memcpy(bytes, &data, sizeof(data));
			// ASSERT(data != 0x06, "x");
			// ASSERT(data != 0x06, "x");
			push_back(bytes[0]);
			push_back(bytes[1]);
		}

		/**
		 * \brief Appends 4 bytes of data to the buffer.
		 * \param data Data to append
		 */
		void append_dword(u32 data) {
			byte bytes[4];
			std::memcpy(bytes, &data, sizeof(data));
			for (int i = 0; i < 4; ++i) {
				// ASSERT(bytes[i] != 0x06, "x");
				push_back(bytes[i]);
			}
		}

		/**
		 * \brief Appends 8 bytes of data to the buffer.
		 * \param data Data to append
		 */
		void append_qword(u64 data) {
			byte bytes[8];
			std::memcpy(bytes, &data, sizeof(data));
			for (int i = 0; i < 8; ++i) {
				// ASSERT(bytes[i] != 0x4d, "x");
				push_back(bytes[i]);
			}
		}

		/**
		 * \brief Appends sizeof(\b type) bytes of data to the buffer.
		 * \tparam type Type of the data to append
		 * \param value Data to append
		 */
		template<typename type>
		void append_type(const type& value) {
			const auto bytes = reinterpret_cast<const byte*>(&value);
			insert(end(), bytes, bytes + sizeof(type));
		}

		/**
		 * \brief Appends \b count 0 bytes to the buffer.
		 * \param count Number of 0 bytes to append
		 */
		void append_zero(u64 count) {
			reserve(m_size + count);
			memset(m_data + m_size, 0, count);
			m_size += count;
		}

		/**
		 * \brief Appends a string without null terminating it.
		 * \param value String to append
		 */
		void append_string(const std::string& value) {
			const auto bytes = reinterpret_cast<const byte*>(value.data());
			insert(end(), bytes, bytes + value.size());
		}

		/**
		 * \brief Appends a null terminated string.
		 * \param value String to append
		 */
		void append_string_nt(const std::string& value) {
			const auto bytes = reinterpret_cast<const byte*>(value.data());
			insert(end(), bytes, bytes + value.size());
			push_back(0);
		}

		/**
		 * \brief Retrieves the byte located at the specified \b index.
		 * \param index Index to retrieve
		 * \return Byte located at the given index.
		 */
		auto get_byte(u64 index) const -> u8 {
			return m_data[index];
		}

		/**
		 * \brief Retrieves the word located at the specified byte \b index.
		 * \param index Index to retrieve
		 * \return Word located at the given index.
		 */
		auto get_word(u64 index) const -> u16 {
			u16 result;
			std::memcpy(&result, &m_data[index], sizeof(result));
			return result;
		}

		/**
		 * \brief Retrieves the dword located at the specified byte \b index.
		 * \param index Index to retrieve
		 * \return Dword located at the given index.
		 */
		auto get_dword(u64 index) const -> u32 {
			u32 result;
			std::memcpy(&result, &m_data[index], sizeof(result));
			return result;
		}

		/**
		 * \brief Retrieves the qword located at the specified byte \b index.
		 * \param index Index to retrieve
		 * \return Qword located at the given index.
		 */
		auto get_qword(u64 index) const -> u64 {
			u64 result;
			std::memcpy(&result, &m_data[index], sizeof(result));
			return result;
		}

		/**
		 * \brief Patches the byte at the specified \b pos with the given \b data.
		 * \param pos Pos of the data to patch
		 * \param data Data to patch with
		 */
		void patch_byte(u64 pos, u8 data) const {
			*reinterpret_cast<u8*>(&m_data[pos]) = data;
		}

		/**
		 * \brief Patches the word at the specified \b pos with the given \b data.
		 * \param pos Pos of the data to patch
		 * \param data Data to patch with
		 */
		void patch_word(u64 pos, u16 data) const {
			*reinterpret_cast<u16*>(&m_data[pos]) = data;
		}

		/**
		 * \brief Patches the dword at the specified \b pos with the given \b data.
		 * \param pos Pos of the data to patch
		 * \param data Data to patch with
		 */
		void patch_dword(u64 pos, u32 data) const {
			*reinterpret_cast<u32*>(&m_data[pos]) = data;
		}

		/**
		 * \brief Patches the qword at the specified \b pos with the given \b data.
		 * \param pos Pos of the data to patch
		 * \param data Data to patch with
		 */
		void patch_qword(u64 pos, u64 data) const {
			*reinterpret_cast<u64*>(&m_data[pos]) = data;
		}

		/**
		 * \brief Resolves a chain of 32-bit relocations.
		 * \param head Pointer to the head of the relocation chain
		 * \param target Target address, to which the new relocations should point to
		 */
		void resolve_relocation_dword(u32* head, u32 target) const {
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
		void emit_relocation_dword(u32* head, u32 pos) const {
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
} // namespace utility
