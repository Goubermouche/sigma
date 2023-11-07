#pragma once
#include <utility/types.h>

namespace ir::cg {
	using namespace utility::types;

	class reg {
	public:
		class classified_reg;

		using id_type = u8;
		
		reg() : m_id(invalid_id) {}
		reg(id_type id) : m_id(id) {}

		bool operator==(reg other) const {
			return m_id == other.m_id;
		}
		
		bool is_valid() const {
			return m_id != invalid_id;
		}

		id_type get_id() const {
			return m_id;
		}

		void set_id(id_type id) {
			m_id = id;
		}

		static constexpr id_type invalid_id = std::numeric_limits<id_type>::max();
	protected:
		id_type m_id;
	};

	class classified_reg : public reg {
	public:
		using class_type = u8;

		classified_reg() : m_class(invalid_class) {}
		explicit classified_reg(id_type id, class_type c)
			: reg(id), m_class(c) {}

		bool operator==(classified_reg other) const {
			return m_id == other.m_id && m_class == other.m_class;
		}

		class_type get_class() const {
			return m_class;
		}

		static constexpr class_type invalid_class = std::numeric_limits<class_type>::max();
	protected:
		class_type m_class;
	};
}
