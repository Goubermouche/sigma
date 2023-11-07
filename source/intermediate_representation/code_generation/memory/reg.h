#pragma once
#include <utility/types.h>

namespace ir::cg {
	using namespace utility::types;

	class reg {
	public:
		using id_type = u8;
		
		reg();
		reg(id_type id);

		bool operator==(reg other) const;
		
		bool is_valid() const;

		id_type get_id() const;
		void set_id(id_type id);

		static constexpr id_type invalid_id = std::numeric_limits<id_type>::max();
	protected:
		id_type m_id;
	};

	class classified_reg : public reg {
	public:
		using class_type = u8;

		classified_reg();
		classified_reg(id_type id, class_type c);
			
		bool operator==(classified_reg other) const;

		class_type get_class() const;

		static constexpr class_type invalid_class = std::numeric_limits<class_type>::max();
	protected:
		class_type m_class;
	};
}
