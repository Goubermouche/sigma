#pragma once
#include <utility/types.h>

namespace ir {
	class symbol {
	public:
		enum class tag {
			none,

			// symbol is dead
			tombstone,

			external,
			global,
			function,

			max
		};

		symbol(const std::string& name, tag tag);

		const std::string& get_name() const;
		void set_name(const std::string& name);

		tag get_tag() const;
		void set_tag(tag tag);
	private:
		std::string m_name;
		tag m_tag;
	};
}