#include "program_options.h"

namespace sigma {
	program_options::program_options() {
		add_command(
			"help", {
				.description = "show help message",
				.action = [&](argument_list& arguments) -> i32 {
					SUPPRESS_C4100(arguments);
					display_help();
					return 0;
				}
			}
		);
	}

	command& program_options::add_command(
		const std::string& name, 
		const command_desc& desc
	) {
		return m_commands[name] = command(name, desc);
	}

	i32 program_options::parse(
		i32 argc, 
		char* argv[]
	) {
		// 1 argument is considered as empty since we ignore the .exe path argument
		if (argc == 1) {
			display_help();
			return 1;
		}

		m_application_name = std::filesystem::path(argv[0]).stem().string();

		const auto command = argv[1];
		const auto command_it = m_commands.find(command);

		if (command_it == m_commands.end()) {
			utility::console::out << "cannot find command '" << command << "'\n";
			return 1;
		}

		// the provided vector does not contain the first (.exe path) and
		// second (command tag) arguments
		const std::vector<std::string> arguments(argv + 2, argv + argc);

		// check for a help directive
		if (std::any_of(arguments.begin(), arguments.end(), is_help)) {
			command_it->second.display_help();
			return 0;
		}

		if (const i32 result = command_it->second.parse(arguments)) {
			return result;
		}

		// verify that all required arguments were provided
		std::vector<ptr<argument>> missing_arguments;

		command_it->second.get_arguments().traverse_ordered(
			[&](const std::string& key, const ptr<argument>& argument) {
				SUPPRESS_C4100(key);

				if (argument->is_required() && argument->is_used() == false) {
					missing_arguments.emplace_back(argument);
				}
			}
		);

		// print missing arguments
		if (!missing_arguments.empty()) {
			utility::console::out
				<< missing_arguments.size()
				<< " missing "
				<< (missing_arguments.size() == 1 ? "argument" : "arguments")
				<< " detected:\n";

			for (const auto& missing_argument : missing_arguments) {
				utility::console::out << "   " << missing_argument->get_long_tag();

				if (!missing_argument->get_short_tag().empty()) {
					utility::console::out << " (" << missing_argument->get_short_tag() << ')';
				}

				utility::console::out << '\n';
			}

			return 1;
		}

		return command_it->second.call_action();
	}

	void program_options::display_help() const {
		// usage section
		utility::console::out << "usage: " << m_application_name << " {";

		std::string separator = "";
		for (const auto& [command_tag, command] : m_commands) {
			utility::console::out << separator << command_tag;
			separator = ", ";
		}

		utility::console::out << "}\n\n";

		// print individual commands
	   // find the largest command tag
		u64 max_length = 0;
		for (const auto& [command_tag, command] : m_commands) {
			max_length = std::max(max_length, command_tag.size());
		}

		// individual commands
		utility::console::out << "commands: \n";
		for (const auto& [command_tag, command] : m_commands) {
			utility::console::out
				<< "  " << utility::console::left << utility::console::width(max_length + 3)
				<< command_tag << command.get_description() << '\n';
		}
	}
}