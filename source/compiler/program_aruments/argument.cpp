#include "argument.h"

namespace sigma {
	bool is_help(const std::string& str) {
		return str == "-h" || str == "--help";
	}

	argument::argument(
		const std::string& long_tag,
		const argument_desc& desc
	) : m_short_tag(desc.short_tag.empty() ? "" : SHORT_TAG(desc.short_tag)),
	m_long_tag(long_tag),
	m_description(desc.description),
	m_expected(desc.expected),
	m_is_required(desc.required),
	m_default_value(desc.default_value),
	m_implicit_value(desc.implicit_value),
	m_value_range(desc.value_range) {
		if (m_implicit_value.has_value()) {
			m_value_range = { 0, 0 };
		}
	}

	bool argument::is_required() const {
		return m_is_required;
	}

	bool argument::is_used() const {
		return m_is_used;
	}

	const std::string& argument::get_short_tag() const {
		return m_short_tag;
	}

	const std::string& argument::get_long_tag() const {
		return m_long_tag;
	}

	const std::string& argument::get_expected() const {
		return m_expected;
	}

	const std::string& argument::get_description() const {
		return m_description;
	}

	i32 argument::parse(
		std::vector<std::string>::const_iterator& start, 
		std::vector<std::string>::const_iterator end, 
		const std::string& specified_tag
	) {
		// check if the argument is used
		if (m_is_used) {
			utility::console::out << "argument '" << specified_tag << "' has already been defined before\n";
			return 1;
		}

		m_is_used = true;

		if (m_value_range.max == 0) {
			m_values.emplace_back(m_implicit_value);
			return 0;
		}

		// check that we have enough remaining arguments
		if (static_cast<u64>(std::distance(start, end)) >= m_value_range.min) {
			u64 parsed_argument_value_count = 0;

			while (start != end) {
				const auto current_value = *start;

				// check if the potential value isn't a new tag
				if (current_value.starts_with('-')) {
					// if it is a tag, we should stop parsing
					break;
				}

				m_values.emplace_back(current_value);

				// check if we haven't exceeded our max argument range
				if (parsed_argument_value_count == m_value_range.max) {
					utility::console::out << "too many argument values passed for argument '" << specified_tag << "'\n";
					return 1;
				}

				parsed_argument_value_count++;
				start++;
			}
		}

		// we've finished parsing
		// first we should check if we've parsed enough values
		if (m_values.size() < m_value_range.min) {
			utility::console::out << "too few argument values passed for argument '" << specified_tag << "'\n";
			return 1;
		}

		return 0;
	}

	void argument_list::add_argument(
		const std::string& long_name,
		const argument_desc& desc
	) {
		const std::string long_tag = LONG_TAG(long_name);
		const auto arg = std::make_shared<argument>(long_tag, desc);

		m_arguments[long_tag] = arg;

		if (!desc.short_tag.empty()) {
			m_arguments[SHORT_TAG(desc.short_tag)] = arg;
		}

		m_insertion_order.push_back(long_tag);
	}

	void argument_list::traverse_ordered(const std::function<void(
		const std::string&, 
		const s_ptr<argument>&)>& visitor
	) const {
		for (const auto& key : m_insertion_order) {
			visitor(key, m_arguments.at(key));
		}
	}

	bool argument_list::empty() const {
		return m_arguments.empty();
	}

	std::unordered_map<std::string, s_ptr<argument>>::iterator argument_list::find(
		const std::string& key
	) {
		return m_arguments.find(key);
	}

	s_ptr<argument>& argument_list::operator[](const std::string& key) {
		return m_arguments[key];
	}

	std::unordered_map<std::string, s_ptr<argument>>::iterator argument_list::begin() {
		return m_arguments.begin();
	}

	std::unordered_map<std::string, s_ptr<argument>>::iterator argument_list::end() {
		return m_arguments.end();
	}
}
