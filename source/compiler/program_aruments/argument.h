#pragma once
#include <utility/macros.h>
#include <utility/diagnostics/error.h>

using namespace utility::types;

namespace sigma {
#define SHORT_TAG(value) '-' + value
#define LONG_TAG(value) "--" + value

	struct range {
		u64 min;
		u64 max;
	};

	bool is_help(const std::string& str);

	struct argument_desc {
		std::string short_tag;
		std::string description;
		std::string expected;

		bool required = true;

		std::any default_value;
		std::any implicit_value;

		range value_range = { 1, 1 };
	};

	class argument {
	public:
		argument() = default;
		argument(
			const std::string& long_tag,
			const argument_desc& desc
		);

		bool is_required() const;
		bool is_used() const;
		const std::string& get_short_tag() const;
		const std::string& get_long_tag() const;
		const std::string& get_expected() const;
		const std::string& get_description() const;

		template<typename type>
		static type any_cast_container(const std::vector<std::any>& values);

		template<typename type>
		type get() const;

		i32 parse(
			std::vector<std::string>::const_iterator& start,
			std::vector<std::string>::const_iterator end,
			const std::string& specified_tag
		);
	private:
		std::string m_short_tag;
		std::string m_long_tag;
		std::string m_description;
		std::string m_expected;

		bool m_is_required;
		bool m_is_used = false;

		std::any m_default_value;
		std::any m_implicit_value;

		std::vector<std::any> m_values;

		range m_value_range;
	};

	template<typename type>
	inline type argument::any_cast_container(
		const std::vector<std::any>& values
	) {
		// convert the entire values vector into the specified container
		// with the given value type
		using value_type = typename type::value_type;

		type result;
		std::transform(
			values.begin(),
			values.end(),
			std::back_inserter(result),
			[](const auto& value) {
				return std::any_cast<value_type>(value);
			}
		);

		return result;
	}

	template<typename type>
	inline type argument::get() const {
		if (!m_values.empty()) {
			if constexpr (std::is_same_v<type, std::string>) {
				return std::any_cast<type>(m_values.front());
			}

			// check if the type is a container
			if constexpr (utility::detail::is_container<type>()) {
				return any_cast_container<type>(m_values);
			}

			utility::console::out << "HER�\n";
			return std::any_cast<type>(m_values.front());
		}

		if (m_default_value.has_value()) {
			if constexpr (utility::detail::is_container<type>()) {
				return any_cast_container<type>({ m_default_value });
			}

			return std::any_cast<type>(m_default_value);
		}

		ASSERT(false, "no value provided for argument");
		return type();
	}

	class argument_list {
	public:
		void add_argument(
			const std::string& long_name,
			const argument_desc& desc
		);

		void traverse_ordered(
			const std::function<void(const std::string&, const ptr<argument>&)>& visitor
		) const;

		template<typename type>
		type get(const std::string& name) const;

		bool empty() const;
		std::unordered_map<std::string, ptr<argument>>::iterator find(const std::string& key);
		ptr<argument>& operator[](const std::string& key);

		std::unordered_map<std::string, ptr<argument>>::iterator begin();
		std::unordered_map<std::string, ptr<argument>>::iterator end();
	private:
		std::vector<std::string> m_insertion_order;
		std::unordered_map<std::string, ptr<argument>> m_arguments;
	};

	template<typename type>
	inline type argument_list::get(const std::string& name) const {
		// look for a long tag
		auto argument_it = m_arguments.find(LONG_TAG(name));
		if (argument_it != m_arguments.end()) {
			return argument_it->second->get<type>();
		}

		// look for a short tag
		argument_it = m_arguments.find(SHORT_TAG(name));
		if (argument_it != m_arguments.end()) {
			return argument_it->second->get<type>();
		}

		ASSERT(false, "cannot find argument with name '" + name + "'");
		return type();
	}
}
