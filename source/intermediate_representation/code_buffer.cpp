#include "code_buffer.h"

namespace ir {
	u64 code_buffer::get_size() const {
		return m_code.size();
	}

	void code_buffer::append_byte(u8 data) {
		m_code.emplace_back(data);
	}

	void code_buffer::append_word(u16 data) {
		utility::byte bytes[2];
		std::memcpy(bytes, &data, sizeof(data));
		m_code.emplace_back(bytes[0]);
		m_code.emplace_back(bytes[1]);
	}

	void code_buffer::append_dword(u32 data) {
		utility::byte bytes[4];
		std::memcpy(bytes, &data, sizeof(data));
		for (int i = 0; i < 4; ++i) {
			m_code.emplace_back(bytes[i]);
		}
	}

	void code_buffer::append_qword(u64 data) {
		utility::byte bytes[8];
		std::memcpy(bytes, &data, sizeof(data));
		for (int i = 0; i < 8; ++i) {
			m_code.emplace_back(bytes[i]);
		}
	}

	void code_buffer::patch_dword(u64 pos, u32 data) {
		*reinterpret_cast<uint32_t*>(&m_code[pos]) = data;
	}

	void code_buffer::resolve_relocation_dword(u32* head, u32 target) {
		u32 current = *head;

		// walk previous relocations
		while (current != 0 && (current & 0x80000000) == 0) {
			const u32 next = *reinterpret_cast<u32*>(&m_code[current]);
			patch_dword(current, target - (current + 4));
			current = next;
		}

		// store the target and mark it as resolved
		*head = 0x80000000 | target;
	}

	void code_buffer::emit_relocation_dword(u32* head, u32 pos) {
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

	code_buffer::array_type::iterator code_buffer::begin() {
		return m_code.begin();
	}

	code_buffer::array_type::iterator code_buffer::end() {
		return m_code.end();
	}

	code_buffer::array_type::const_iterator code_buffer::begin() const {
		return m_code.cbegin();
	}

	code_buffer::array_type::const_iterator code_buffer::end() const {
		return m_code.cend();
	}
}