#pragma once 

namespace utility {
	template<typename type>
	class iterator {
	public:
		iterator(type* m_ptr) : m_ptr(m_ptr) {}

		iterator& operator++() {
			++m_ptr;
			return *this;
		}

		iterator operator++(int) {
			iterator temp = *this;
			++m_ptr;
			return temp;
		}

		type& operator*() {
			return *m_ptr;
		}

		bool operator==(const iterator& other) const {
			return m_ptr == other.m_ptr;
		}

		bool operator!=(const iterator& other) const {
			return m_ptr != other.m_ptr;
		}
	private:
		type* m_ptr;
	};
}
