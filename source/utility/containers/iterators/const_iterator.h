#pragma once

namespace utility {
	template<typename type>
	class const_iterator {
	public:
		const_iterator(const type* m_ptr) : m_ptr(m_ptr) {}

		const_iterator& operator++() {
			++m_ptr;
			return *this;
		}

		const_iterator operator++(int) {
			const_iterator temp = *this;
			++m_ptr;
			return temp;
		}

		const type& operator*() const {
			return *m_ptr;
		}

		bool operator==(const const_iterator& other) const {
			return m_ptr == other.m_ptr;
		}

		bool operator!=(const const_iterator& other) const {
			return m_ptr != other.m_ptr;
		}
	private:
		const type* m_ptr;
	};
}
