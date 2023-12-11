#include "command.h"

namespace sigma {
	command::command(
		const std::string& name,
		const command_desc& desc
	) : m_name(name),
	m_description(desc.description),
	m_action(desc.action) {}

	void command::add_argument(
		const std::string& long_name,
		const argument_desc& desc
	) {
		m_arguments.add_argument(long_name, desc);
	}

	const std::string& command::get_description() const {
		return m_description;
	}

	void command::display_help() {
		// print usage
		constexpr u64 max_line_length = 90;
		const std::string help_intro_string = "[--help]";
		const  std::string intro = "usage: " + m_name + " " + help_intro_string;
		const u64 first_arg_padding = intro.length() - help_intro_string.size() - 1;

		std::string current_line = intro;

		m_arguments.traverse_ordered(
			[&](const std::string& key, const s_ptr<argument>& argument) {
				SUPPRESS_C4100(key);

				std::stringstream argument_ss;
				argument_ss << " [" << argument->get_long_tag();

				if (!argument->get_expected().empty()) {
					argument_ss << "=<" << argument->get_expected() << '>';
				}

				argument_ss << ']';

				std::string argument_str = argument_ss.str();

				// if adding this argument would make the line too long, print the current line and start a new one.
				if (current_line.length() + 1 + argument_str.length() > max_line_length) {
					utility::console::print("{}\n", current_line);
					current_line = std::string(first_arg_padding, ' ') + argument_str;
				}
				else {
					// if not, just add it to the current line.
					current_line += argument_str;
				}
			}
		);

		// print any remaining content on the current line.
		if (!current_line.empty()) {
			utility::console::print("{}\n", current_line);
		}

		utility::console::print("\n");

		if (!m_description.empty()) {
			utility::console::print("{}\n\n", m_description);
		}

		// print individual arguments
		 // find the longest short/long tag pair
		u64 max_length = 0;
		for (const auto& [argument_tag, argument] : m_arguments) {
			max_length = std::max(max_length, argument->get_short_tag().size() + argument->get_long_tag().size());
		}

		// required arguments
		utility::console::print("required arguments:\n");
		m_arguments.traverse_ordered(
			[max_length](const std::string& key, const s_ptr<argument>& argument) {
				SUPPRESS_C4100(key);

				if (argument->is_required()) {
					std::stringstream tag_section;
					tag_section << "   " << argument->get_long_tag();

					if (!argument->get_short_tag().empty()) {
						tag_section << ", " << argument->get_short_tag();
					}

					utility::console::print("{:<{}}{}\n", tag_section.str(), max_length + 8, argument->get_description());
				}
			}
		);

		// optional arguments
		utility::console::print("\noptional arguments:\n");
		utility::console::print("{:<{}} show help message\n", "   --help, -h", max_length + 8);

		m_arguments.traverse_ordered(
			[max_length](const std::string& key, const s_ptr<argument>& argument) {
				SUPPRESS_C4100(key);

				if (argument->is_required() == false) {
					std::stringstream tag_section;
					tag_section << "   " << argument->get_long_tag();

					if (!argument->get_short_tag().empty()) {
						tag_section << ", " << argument->get_short_tag();
					}

					utility::console::print("{:<{}}{}\n", tag_section.str(), max_length + 8, argument->get_description());
				}
			}
		);
	}

	i32 command::parse(const std::vector<std::string>& arguments) {
		if (arguments.empty() && !m_arguments.empty()) {
			display_help();
			return 1;
		}

		const auto end = arguments.end();
		for (auto it = arguments.begin(); it != end;) {
			const auto current_argument_tag = *it++;
			const auto argument_it = m_arguments.find(current_argument_tag);

			if (argument_it == m_arguments.end()) {
				utility::console::print(
					"cannot find argument '{}' within the '{}' command",
					current_argument_tag, 
					m_name
				);

				return 1;
			}

			if (const i32 result = argument_it->second->parse(it, end, current_argument_tag)) {
				return result;
			}
		}

		return 0;
	}

	const argument_list& command::get_arguments() const {
		return m_arguments;
	}

	i32 command::call_action() {
		if (m_action != nullptr) {
			return m_action(m_arguments);
		}

		return 0;
	}
}