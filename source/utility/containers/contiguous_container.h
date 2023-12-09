#pragma once
#include "utility/macros.h"
#include "utility/containers/slice.h"

namespace utility {
	template<typename type>
	class contiguous_container {
	public:
		constexpr contiguous_container()
			: m_owning(true), m_data(nullptr), m_size(0), m_capacity(0) {}

		constexpr contiguous_container(u64 size)
			: m_owning(true), m_data(allocate(size)), m_size(size), m_capacity(size) {}

		constexpr contiguous_container(u64 size, type* data)
			: m_owning(false), m_data(data), m_size(size), m_capacity(size) {}

		contiguous_container(const contiguous_container& other)
			: m_owning(true), m_size(other.m_size), m_capacity(other.m_capacity) {
			m_data = static_cast<type*>(std::malloc(m_capacity * sizeof(type)));
			std::memcpy(m_data, other.m_data, m_capacity * sizeof(type));
		}

		[[nodiscard]] contiguous_container& operator=(const contiguous_container& other) {
			if(&other == this) {
				return *this;
			}

			m_owning = true;
			m_size = other.m_size;
			m_capacity = other.m_capacity;
			m_data = static_cast<type*>(std::malloc(m_capacity * sizeof(type)));
			std::memcpy(m_data, other.m_data, m_capacity * sizeof(type));

			return *this;
		}

		constexpr ~contiguous_container() {
			if (m_owning) {
				if (!std::is_trivial_v<type>) {
					destruct_range(begin(), end());
				}

				std::free(m_data);
				m_data = nullptr;
			}
		}

		[[nodiscard]] static contiguous_container zero_initialize(u64 size) {
			const contiguous_container container(size);
			container.zero_fill();
			return container;
		}

		constexpr void clear() {
			m_size = 0;
			zero_fill();
		}

		[[nodiscard]] constexpr  auto empty() const -> bool {
			return m_size == 0;
		}

		constexpr void zero_fill() const {
			if(m_data) {
				std::memset(m_data, 0, m_size * sizeof(type));
			}
		}

		void push_back(const type& value)
		{
			if (m_size == m_capacity) {
				grow();
			}

			if constexpr (std::is_trivial_v<type>) {
				m_data[m_size] = value;
			}
			else {
				new (m_data + m_size) type(value);
			}

			m_size++;
		}

		constexpr void push_back(type&& value) {
			ASSERT(m_owning, "non-owning container cannot change the memory layout");

			if (m_size == m_capacity) {
				grow();
			}

			if constexpr (std::is_trivial_v<type>) {
				m_data[m_size] = value;
			}
			else {
				 new (m_data + m_size) type(std::move(value));
			}

			m_size++;
		}

		template<typename... arg_types>
		constexpr void emplace_back(arg_types&&... args) {
			ASSERT(m_owning, "non-owning container cannot change the memory layout");

			static_assert(
				!std::is_trivial_v<type>, 
				"use push_back() instead of emplace_back() with trivial types"
			);

			if (m_size == m_capacity) {
				grow();
			}

			new (m_data + m_size) type(std::forward<arg_types>(args)...);
			m_size++;
		}

		constexpr void insert(type* where, const type* start, const type* end) {
			ASSERT(m_owning, "non-owning container cannot change the memory layout");
			const u64 elements_to_insert = end - start;

			if (elements_to_insert == 0) {
				return;
			}

			u64 insert_index = where - m_data;

			if (m_size + elements_to_insert > m_capacity) {
				reserve(m_size + elements_to_insert);

				// update where after reserve as it might have changed
				where = m_data + insert_index;
			}

			// move existing elements to make space for new elements
			if (m_size > insert_index) {
				if constexpr (std::is_trivially_move_constructible_v<type> && std::is_trivially_destructible_v<type>) {
					std::memmove(where + elements_to_insert, where, (m_size - insert_index) * sizeof(type));
				}
				else {
					for (u64 i = m_size; i > insert_index; --i) {
						new (m_data + i + elements_to_insert - 1) type(std::move(m_data[i - 1]));
						m_data[i - 1].~type();
					}
				}
			}

			// insert new elements
			for (u64 i = 0; i < elements_to_insert; ++i) {
				if constexpr (std::is_trivially_copyable_v<type>) {
					where[i] = start[i];
				}
				else {
					new (where + i) type(start[i]);
				}
			}

			m_size += elements_to_insert;
		}

		constexpr void append(const type* source_begin, const type* source_end) {
			insert(end(), source_begin, source_end);
		}

		constexpr void append(const contiguous_container& other) {
			insert(end(), other.begin(), other.end());
		}

		constexpr void prepend(const contiguous_container& other) {
			insert(begin(), other.begin(), other.end());
		}

		type& operator[](u64 index) {
			return m_data[index];
		}

		const type& operator[](u64 index) const {
			return m_data[index];
		}

		[[nodiscard]] constexpr auto get_slice(u64 start, u64 size) const -> slice<type> {
			return { m_data + start, size };
		}

		[[nodiscard]] constexpr auto get_size() const -> u64 {
			return m_size;
		}

		[[nodiscard]] constexpr auto get_capacity() const -> u64 {
			return m_capacity;
		}

		[[nodiscard]] constexpr auto begin() -> type* {
			return m_data;
		}

		[[nodiscard]] constexpr auto begin() const -> const type* {
			return m_data;
		}

		[[nodiscard]] constexpr auto end() -> type* {
			return m_data + m_size;
		}

		[[nodiscard]] constexpr auto end() const -> const type* {
			return m_data + m_size;
		}

		[[nodiscard]] constexpr auto first() const -> const type& {
			return m_data[0];
		}

		[[nodiscard]] constexpr auto last() const -> const type& {
			return m_data[m_size - 1];
		}

		constexpr void reserve(u64 capacity) {
			ASSERT(m_owning, "non-owning container cannot change the memory layout");
			ASSERT(
				capacity > m_capacity,
				"capacity is already equal to or greater than the passed value"
			);

			if constexpr (std::is_trivial_v<type>) {
				m_data = static_cast<type*>(std::realloc(m_data, sizeof(type) * capacity));
				ASSERT(m_data != nullptr, "reallocation failed");
			}
			else {
				type* new_data = static_cast<type*>(std::malloc(sizeof(type) * capacity));
				ASSERT(new_data != nullptr, "allocation failed");

				copy_range(begin(), end(), new_data);
				destruct_range(begin(), end());

				std::free(m_data);
				m_data = new_data;
			}

			m_capacity = capacity;
		}

		constexpr void resize(u64 size) {
			ASSERT(m_owning, "non-owning container cannot change the memory layout");
			ASSERT(size != m_size, "size is already equal to the passed value");

			if (size > m_capacity) {
				reserve(size);
			}

			if constexpr (!std::is_trivial_v<type>) {
				if (size > m_size) {
					construct_range(m_data + m_size, m_data + size);
				}
				else {
					destruct_range(m_data + size, m_data + m_size);
				}
			}

			m_size = size;
		}

		constexpr void resize(u64 size, const type& value) {
			ASSERT(m_owning, "non-owning container cannot change the memory layout");
			ASSERT(size != m_size, "size is already equal to the passed value");

			if (size > m_capacity) {
				reserve(size);
			}

			if constexpr (!std::is_trivial_v<type>) {
				if (size > m_size) {
					for (auto p = m_data + m_size; p != m_data + size; ++p) {
						new (p) type(value); // Construct each new element with the given value
					}
				}
				else {
					destruct_range(m_data + size, m_data + m_size);
				}
			}
			else {
				if (size > m_size) {
					std::fill(m_data + m_size, m_data + size, value);
				}
			}

			m_size = size;
		}
	protected:
		static type* allocate(u64 count) {
			return static_cast<type*>(std::malloc(count * sizeof(type)));
		}
		void grow() {
			ASSERT(m_owning, "non-owning container cannot change the memory layout");
			reserve(m_capacity * 2 + 1);
		}

		static constexpr void destruct_range(type* begin, type* end) {
			while (begin != end) {
				begin->~type();
				++begin;
			}
		}

		static constexpr void copy_range(type* begin, type* end, type* destination) {
			while (begin != end) {
				new (destination) type(*begin);
				++begin;
				++destination;
			}
		}

		static constexpr void construct_range(type* begin, type* end) {
			while (begin != end) {
				new (begin) type;
				++begin;
			}
		}
	protected:
		bool m_owning;
		type* m_data;
		u64 m_size;
		u64 m_capacity;
	};
}
