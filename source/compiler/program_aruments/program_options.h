#pragma once
#include "compiler/program_aruments/command.h"

namespace sigma {
	class program_options {
	public:
		program_options();

		command& add_command(
			const std::string& name,
			const command_desc& desc
		);

		i32 parse(
			i32 argc,
			char* argv[]
		);
	private:
		void display_help() const;
	private:
		std::unordered_map<std::string, command> m_commands;
		std::string m_application_name;
	};
}
