#pragma once
#include "compiler/program_aruments/argument.h"

namespace sigma {
	using action_type = std::function<i32(argument_list&)>;

	struct command_desc {
		std::string description;
		action_type action;
	};

	class command {
	public:
		command() = default;
		command(
			const std::string& name,
			const command_desc& desc
		);

		void add_argument(
			const std::string& long_name,
			const argument_desc& desc
		);

		const std::string& get_description() const;

		void display_help();

		i32 parse(
			const std::vector<std::string>& arguments
		);

		const argument_list& get_arguments() const;

		i32 call_action();
	private:
		std::string m_name;
		std::string m_description;

		argument_list m_arguments;
		action_type m_action;
	};
}