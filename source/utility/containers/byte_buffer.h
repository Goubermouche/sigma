#pragma once
#include <utility/memory.h>

namespace utility {
	/**
	 * \brief Simple abstraction for a list of bytes. Provides basic quality of life
	 * features such as resolving relocations and iterators.
	 */
	class byte_buffer {
	public:
		using array_type = std::vector<utility::byte>;

		utility::byte* get_data() {
			return m_bytes.data();
		}

		const utility::byte* get_data() const {
			return m_bytes.data();
		}

		u64 get_size() const {
			return m_bytes.size();
		}

		void append_byte(u8 data) {
			m_bytes.emplace_back(data);
		}

		void append_word(u16 data) {
			utility::byte bytes[2];
			std::memcpy(bytes, &data, sizeof(data));
			m_bytes.emplace_back(bytes[0]);
			m_bytes.emplace_back(bytes[1]);
		}

		void append_dword(u32 data) {
			utility::byte bytes[4];
			std::memcpy(bytes, &data, sizeof(data));
			for (int i = 0; i < 4; ++i) {
				m_bytes.emplace_back(bytes[i]);
			}
		}

		void append_qword(u64 data) {
			utility::byte bytes[8];
			std::memcpy(bytes, &data, sizeof(data));
			for (int i = 0; i < 8; ++i) {
				m_bytes.emplace_back(bytes[i]);
			}
		}

		void patch_dword(u64 pos, u32 data) {
			*reinterpret_cast<uint32_t*>(&m_bytes[pos]) = data;
		}

		/**
		 * \brief Resolves a chain of 32-bit relocations.
		 * \param head Pointer to the head of the relocation chain
		 * \param target Target address, to which the new relocations should point to
		 */
		void resolve_relocation_dword(u32* head, u32 target) {
			u32 current = *head;

			// walk previous relocations
			while (current != 0 && (current & 0x80000000) == 0) {
				const u32 next = *reinterpret_cast<u32*>(&m_bytes[current]);
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
		void emit_relocation_dword(u32* head, u32 pos) {
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

		array_type::iterator begin() {
			return m_bytes.begin();
		}

		array_type::iterator end() {
			return m_bytes.end();
		}

		array_type::const_iterator begin() const {
			return m_bytes.cbegin();
		}

		array_type::const_iterator end() const {
			return m_bytes.cend();
		}
	private:
		array_type m_bytes;
	};
}