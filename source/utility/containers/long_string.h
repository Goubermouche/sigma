#pragma once
#include "utility/types.h"
#include "utility/containers/iterators/iterator.h"
#include "utility/containers/iterators/const_iterator.h"

#define LONG_STRING_BLOCK_SIZE 256ull

namespace utility {
	class long_string {
	public:
		long_string() = default;

		long_string(const char* text) {
			append(text);
		}

		template<typename... arguments>
		long_string(const std::format_string<arguments...> fmt, arguments&&... args) {
			append(std::vformat(fmt.get(), std::make_format_args(args...)).c_str());
		}

		~long_string() {
			for (char* chunk : m_chunks) {
				delete[] chunk;
			}
		}

		void operator+=(const char* text) {
			append(text);
		}

		template<typename... arguments>
		void append(
			const std::format_string<arguments...> fmt, 
			arguments&&... args
		) {
			append(std::vformat(fmt.get(), std::make_format_args(args...)).c_str());
		}

		char* get_chunk(u64 index) {
			return m_chunks[index];
		}

		iterator<char*> begin() {
			return iterator<char*>(&(*m_chunks.begin()));
		}

		iterator<char*> end() {
			return iterator<char*>(&(*m_chunks.end()));
		}

		const_iterator<char*> begin() const {
			return const_iterator<char*>(&(*m_chunks.begin()));
		}

		const_iterator<char*> end() const {
			return const_iterator<char*>(&(*m_chunks.end()));
		}

		void append(const char* text) {
			u64 text_length = std::strlen(text);
			u64 final_text_length = m_size + text_length;
			u64 current_length_cap = m_chunks.size() * LONG_STRING_BLOCK_SIZE;

			// number of characters which we can still fit into our chunks 
			// without allocating new ones
			u64 free_length = current_length_cap - m_size;
			u64 text_start = 0;

			// append to the last block
			if (free_length > 0) {
				u64 chars_to_copy = std::min(
					free_length, text_length
				);

				std::strncpy(
					m_chunks.back() + (LONG_STRING_BLOCK_SIZE - free_length),
					text,
					chars_to_copy
				);

				m_chunks.back()[LONG_STRING_BLOCK_SIZE - free_length + chars_to_copy] = '\0';
				text_start += chars_to_copy;
			}

			// allocate new blocks
			u64 chars_to_allocate = text_length - text_start;
			u64 blocks_to_allocate = (chars_to_allocate + LONG_STRING_BLOCK_SIZE - 1) / LONG_STRING_BLOCK_SIZE;

			u64 old_chunk_count = m_chunks.size();
			m_chunks.resize(old_chunk_count + blocks_to_allocate);

			for (u64 i = old_chunk_count; i < m_chunks.size(); ++i) {
				m_chunks[i] = new char[LONG_STRING_BLOCK_SIZE];

				u64 chars_to_copy;

				if (chars_to_allocate < LONG_STRING_BLOCK_SIZE) {
					chars_to_copy = chars_to_allocate;
				}
				else {
					chars_to_copy = LONG_STRING_BLOCK_SIZE;
					chars_to_allocate -= LONG_STRING_BLOCK_SIZE;
				}

				std::strncpy(m_chunks[i], text + text_start, chars_to_copy);
				text_start += chars_to_copy;

				m_chunks[i][chars_to_copy] = '\0';
			}

			m_size += text_length;
		}
	private:
		std::vector<char*> m_chunks;
		u64 m_size = 0; // size of the string in characters
	};
}
