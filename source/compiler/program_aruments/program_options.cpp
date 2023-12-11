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
			utility::console::print("cannot find command '{}'\n", command);
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
		std::vector<s_ptr<argument>> missing_arguments;

		command_it->second.get_arguments().traverse_ordered(
			[&](const std::string& key, const s_ptr<argument>& argument) {
				SUPPRESS_C4100(key);

				if (argument->is_required() && argument->is_used() == false) {
					missing_arguments.emplace_back(argument);
				}
			}
		);

		// print missing arguments
		if (!missing_arguments.empty()) {
			utility::console::print(
				"{} missing {} detected\n",
				missing_arguments.size(),
				missing_arguments.size() == 1 ? "argument" : "arguments"
			);

			for (const auto& missing_argument : missing_arguments) {
				utility::console::print("   {}", missing_argument->get_long_tag());

				if (!missing_argument->get_short_tag().empty()) {
					utility::console::print(" ({})", missing_argument->get_short_tag());
				}

				utility::console::print("\n");
			}

			return 1;
		}

		return command_it->second.call_action();
	}

	void program_options::display_help() const {
		// usage section
		utility::console::print("usage: {} {{", m_application_name);

		std::string separator = "";
		for (const auto& [command_tag, command] : m_commands) {

			utility::console::print("{}{}", separator, command_tag);
			separator = ", ";
		}

		utility::console::print("}}\n\n");

		// print individual commands
	   // find the largest command tag
		u64 max_length = 0;
		for (const auto& [command_tag, command] : m_commands) {
			max_length = std::max(max_length, command_tag.size());
		}

		// individual commands
		utility::console::print("commands: \n");
		for (const auto& [command_tag, command] : m_commands) {
			utility::console::print("  {:>{}}{}\n", command_tag, max_length + 3, command.get_description());
		}
	}
}