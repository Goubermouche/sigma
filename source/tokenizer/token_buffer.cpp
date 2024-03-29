#include "token_buffer.h"

namespace sigma {
  auto token_buffer::add_token(token_info info) -> u64 {
		m_token_infos.push_back(info);
		return m_token_infos.get_size() - 1;
	}

	auto token_buffer::get_size() const -> u64 {
		return m_token_infos.get_size();
	}

	auto token_buffer::begin() const -> const token_info* {
		return m_token_infos.begin();
	}

	auto token_buffer::end() const ->  const token_info* {
		return m_token_infos.end();
	}

	auto token_buffer::first() const -> token_info {
		return m_token_infos.first();
	}

	auto token_buffer::last() const -> token_info {
		return m_token_infos.last();
	}

	auto token_buffer::empty() const -> bool {
		return m_token_infos.is_empty();
	}

	auto token_buffer::get_token(u64 index) const -> token {
		return m_token_infos[index].tok;
	}

	auto token_buffer::get_token_symbol_key(u64 index) const -> utility::string_table_key {
		return m_token_infos[index].symbol_key;
	}

	auto token_buffer::operator[](u64 index) const -> token_info {
		return m_token_infos[index];
	}

	token_buffer_iterator::token_buffer_iterator(const token_buffer& tokens)
		: m_tokens(tokens), m_current_info(tokens.first()) {}

	void token_buffer_iterator::next() {
		m_index++;
		m_peek_index++;

		ASSERT(m_index < m_tokens.get_size(), "out of bounds token access");

		m_current_info = m_tokens[m_index];
	}

	void token_buffer_iterator::prev() {
		m_index--;
		m_peek_index--;
	}

	auto token_buffer_iterator::peek() -> token_info {
		return m_tokens[++m_peek_index];
	}

	auto token_buffer_iterator::peek_token() -> token {
		return m_tokens[++m_peek_index].tok;
	}

	auto token_buffer_iterator::peek_next() const -> token_info {
		return m_tokens[m_index + 1];
	}

	auto token_buffer_iterator::peek_next_token() const -> token {
		return m_tokens[m_index + 1].tok;
	}

	void token_buffer_iterator::synchronize_indices() {
		m_peek_index = m_index;
	}

	auto token_buffer_iterator::get_current() const -> token_info {
		return m_current_info;
	}

	auto token_buffer_iterator::get_current_token() const -> token {
		return m_current_info.tok;
	}

	auto token_buffer_iterator::get_current_token_location() const -> handle<token_location> {
		return m_current_info.location;
	}

	auto token_buffer_iterator::get_current_token_line_index() const -> u32 {
		return m_current_info.location->line_index;
	}

	auto token_buffer_iterator::get_current_token_char_index() const -> u32 {
		return m_current_info.location->char_index;
	}

	auto token_buffer_iterator::get_current_peek_token() const -> token {
		return m_tokens[m_peek_index].tok;
	}

	auto token_buffer_iterator::get_current_peek() const -> token_info {
		return m_tokens[m_peek_index];
	}
} // namespace sigma::lex
