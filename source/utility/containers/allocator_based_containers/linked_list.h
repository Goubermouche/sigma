#pragma once
#include "utility/containers/handle.h"

namespace utility {
	template<typename type>
	class linked_list {
		struct node {
			type value;
			handle<node> next;
		};
	public:
		class iterator {
			handle<node> current;

		public:
			iterator(handle<node> ptr) : current(ptr) {}

			type& operator*() {
				return current->value;
			}

			iterator& operator++() {
				current = current->next;
				return *this;
			}

			bool operator!=(const iterator& other) const {
				return current != other.current;
			}
		};

		template<typename allocator>
		void append(const type& value, allocator& alloc) {
			handle<node> new_node = allocate_node(alloc);
			new_node->value = value;
			new_node->next = nullptr;

			if (m_first == nullptr) {
				m_first = m_last = new_node;
			}
			else {
				m_last->next = new_node;
				m_last = new_node;
			}

			m_size++;
		}

		type& get_first() {
			return m_first->value;
		}

		type& get_last() {
			return m_last->value;
		}

		const type& get_first() const {
			return m_first->value;
		}

		u64 get_size() const {
			return m_size;
		}

		bool empty() const {
			return m_size == 0;
		}

		iterator begin() {
			return iterator(m_first);
		}

		iterator end() {
			return iterator(nullptr);
		}
	protected:
		template<typename allocator>
		static handle<node> allocate_node(allocator& alloc) {
			return static_cast<node*>(alloc.allocate(sizeof(node)));
		}
	protected:
		handle<node> m_first = nullptr;
		handle<node> m_last = nullptr;

		u64 m_size = 0;
	};
}
