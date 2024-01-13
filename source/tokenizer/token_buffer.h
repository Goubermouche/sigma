#pragma once
#include "tokenizer/token.h"

#include <utility/containers/contiguous_container.h>
#include <utility/containers/string_table.h>

namespace sigma {
	class token_buffer {
	public:
		auto add_token(token_info info) -> u64;

		[[nodiscard]] auto get_size() const -> u64;

		[[nodiscard]] auto get_token(u64 index) const->token;
		[[nodiscard]] auto get_token_symbol_key(u64 index) const -> utility::string_table_key;

		[[nodiscard]] auto operator[](u64 index) const -> token_info;

		[[nodiscard]] auto begin() const -> const token_info*;
		[[nodiscard]] auto end() const -> const token_info*;

		[[nodiscard]] auto first() const->token_info;
		[[nodiscard]] auto last() const -> token_info;

		[[nodiscard]] auto empty() const -> bool;
	private:
		utility::contiguous_container<token_info> m_token_infos;
	};

	class token_buffer_iterator {
	public:
		token_buffer_iterator(const token_buffer& tokens);

		void next();

		void prev();

		auto peek() -> token_info;
		auto peek_token() -> token;

		auto peek_next() const->token_info;
		auto peek_next_token() const -> token;

		void synchronize();

		auto get_current() const-> token_info;
		auto get_current_token() const -> token;
		auto get_current_token_location() const -> handle<token_location>;
		auto get_current_token_line_index() const -> u32;
		auto get_current_token_char_index() const -> u32;
	private:
		const token_buffer& m_tokens;

		token_info m_current_info;

		u64 m_peek_index = 0;
		u64 m_index = 0;
	};
} // namespace sigma
