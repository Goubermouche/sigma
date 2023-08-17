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
			[&](const std::string& key, const std::shared_ptr<argument>& argument) {
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
					console::out << current_line << '\n';
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
			console::out << current_line << '\n';
		}

		console::out << '\n';

		if (!m_description.empty()) {
			console::out << m_description << "\n\n";
		}

		// print individual arguments
		 // find the longest short/long tag pair
		u64 max_length = 0;
		for (const auto& [argument_tag, argument] : m_arguments) {
			max_length = std::max(max_length, argument->get_short_tag().size() + argument->get_long_tag().size());
		}

		// required arguments
		console::out << "required arguments:\n";
		m_arguments.traverse_ordered(
			[max_length](const std::string& key, const std::shared_ptr<argument>& argument) {
				SUPPRESS_C4100(key);

				if (argument->is_required()) {
					std::stringstream tag_section;
					tag_section << "   " << argument->get_long_tag();

					if (!argument->get_short_tag().empty()) {
						tag_section << ", " << argument->get_short_tag();
					}

					console::out
						<< console::left << console::width(max_length + 8)
						<< tag_section.str() << argument->get_description() << '\n';
				}
			}
		);

		// optional arguments
		console::out << "\noptional arguments:\n";
		console::out
			<< console::left << console::width(max_length + 8)
			<< "   --help, -h" << "show help message\n";

		m_arguments.traverse_ordered(
			[max_length](const std::string& key, const std::shared_ptr<argument>& argument) {
				SUPPRESS_C4100(key);

				if (argument->is_required() == false) {
					std::stringstream tag_section;
					tag_section << "   " << argument->get_long_tag();

					if (!argument->get_short_tag().empty()) {
						tag_section << ", " << argument->get_short_tag();
					}

					console::out
						<< console::left << console::width(max_length + 8)
						<< tag_section.str() << argument->get_description() << '\n';
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
				console::out << "cannot find argument '" << current_argument_tag << "' within the '" << m_name << "' command\n";
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