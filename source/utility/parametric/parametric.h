// Parametric: a basic and simple program options parser.
// Made by: @Goubermouche (https://github.com/Goubermouche)

// Copyright(c) 2024 Goubermouche
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <filesystem>
#include <functional>
#include <iostream>
#include <sstream>
#include <format>
#include <any>
#include <map>

#define PARAMETRIC_TABLE_WIDTH 60

namespace parametric {
	constexpr inline int ERROR_RESULT = std::numeric_limits<int>::max();

	// default option parser overloads: 

	template<typename type>
	struct options_parser {
		static auto parse(const std::string& value) -> type {
			return value;
		}
	};

	template<>
	struct parametric::options_parser<float> {
		static auto parse(const std::string& value) -> float {
			return std::stof(value);
		}
	};

	template<>
	struct parametric::options_parser<double> {
		static auto parse(const std::string& value) -> double {
			return std::stof(value);
		}
	};

	template<>
	struct parametric::options_parser<bool> {
		static auto parse(const std::string& value) -> bool {
			if (value == "true") {
				return true;
			}

			if (value == "false") {
				return false;
			}

			return static_cast<bool>(std::stoul(value));
		}
	};

	template<>
	struct parametric::options_parser<uint8_t> {
		static auto parse(const std::string& value) -> uint8_t {
			return static_cast<uint8_t>(std::stoi(value));
		}
	};

	template<>
	struct parametric::options_parser<uint16_t> {
		static auto parse(const std::string& value) -> uint16_t {
			return static_cast<uint16_t>(std::stoi(value));
		}
	};

	template<>
	struct parametric::options_parser<uint32_t> {
		static auto parse(const std::string& value) -> uint32_t {
			return std::stoul(value);
		}
	};

	template<>
	struct parametric::options_parser<uint64_t> {
		static auto parse(const std::string& value) -> uint64_t {
			return std::stoull(value);
		}
	};

	template<>
	struct parametric::options_parser<int8_t> {
		static auto parse(const std::string& value) -> int8_t {
			return static_cast<int8_t>(std::stoi(value));
		}
	};

	template<>
	struct parametric::options_parser<int16_t> {
		static auto parse(const std::string& value) -> int16_t {
			return static_cast<int16_t>(std::stoi(value));
		}
	};

	template<>
	struct parametric::options_parser<int32_t> {
		static auto parse(const std::string& value) -> int32_t {
			return std::stoi(value);
		}
	};

	template<>
	struct parametric::options_parser<int64_t> {
		static auto parse(const std::string& value) -> int64_t {
			return std::stoll(value);
		}
	};

	namespace detail {
		/**
		 * \brief Checks if the specified \b value is a help flag (--help or -h).
		 * \param value Value to check
		 * \return True if the specified \b value is a help flag, false otherwise.
		 */
		inline auto is_help_flag(const std::string& value) -> bool {
			return value == "--help" || value == "-h";
		}

		/**
		 * \brief Prints a block of strings aligned to \b start_offset and limited to a given max width
		 * by \b max_width.
		 * \param strings Strings to print
		 * \param start_offset Start offset to print the strings with
		 * \param max_width Max width of each line in the block
		 */
		inline void print_aligned_strings(const std::vector<std::string>& strings, std::size_t start_offset, std::size_t max_width) {
			std::size_t current_width = 0;

			for (const auto& string : strings) {
				current_width += string.size();

				if (current_width > max_width) {
					// print a newline and align to the beginning of the line above
					std::cout << "\n" << std::string(start_offset, ' ');
					current_width = string.size();
				}

				std::cout << string;
			}
		}

		/**
		 * \brief Checks if a given \b value is a verbose flag (whether it begins with two dashes).
		 * \param value Value to check
		 * \return True if the specified \b value is a verbose flag, false otherwise.
		 */
		inline auto is_verbose_flag(const std::string& value) -> bool {
			// NOTE: this isn't a smart function, but it handles everything correctly for
			//       our needs

			if (value.length() < 2) {
				return false;
			}

			return value[0] == '-' && value[1] == '-';
		}

		/**
		 * \brief Checks if a given \b value is a flag (whether it begins with a dash).
		 * \param value Value to check
		 * \return True if the specified \b value is a flag, false otherwise.
		 */
		inline auto is_flag(const std::string& value) -> bool {
			// NOTE: this isn't a smart function, but it handles everything correctly for
			//       our needs

			if (value.empty()) {
				return false;
			}

			return value[0] == '-';
		}

		/**
		 * \brief Removes \b up \b to \b two leading dashes from a given string.
		 * \param input Input string
		 * \param output Output string
		 * \return True if the operation succeeded (if we were able to remove at most two dashes),
		 * false otherwise (either the string is invalid, or we were able to remove more than two
		 * dashes).
		 */
		inline auto remove_leading_dashes(const std::string& input, std::string& output) -> bool {
			std::size_t dash_count = 0;

			while (dash_count < input.size() && input[dash_count] == '-') {
				++dash_count;
			}

			if (dash_count > 2) {
				return false;
			}

			output = input.substr(dash_count);
			return true;
		}

		/**
		 * \brief A basic abstraction which allows us to have a map with 1 or 2 keys pointing
		 * to the same value.
		 * \tparam key_type Type of the two keys
		 * \tparam value_type Type of the stored value
		 */
		template<typename key_type, typename value_type>
		class dual_map {
			using iterator = typename std::map<key_type, value_type>::iterator;
			using const_iterator = typename std::map<key_type, value_type>::const_iterator;
		public:
			auto insert(const key_type& primary_key, const value_type& value) -> value_type& {
				auto& val_ref = primary_map[primary_key];
				val_ref = value;
				return val_ref;
			}

			auto insert(const key_type& primary_key, const key_type& secondary_key, const value_type& value) -> value_type& {
				auto& val_ref = primary_map[primary_key];
				val_ref = value;
				secondary_to_primary_map[secondary_key] = primary_key;
				return val_ref;
			}

			auto begin() -> iterator { return primary_map.begin(); }
			auto end() -> iterator { return primary_map.end(); }

			auto begin() const -> const_iterator { return primary_map.begin(); }
			auto end() const -> const_iterator { return primary_map.end(); }

			auto cbegin() const -> const_iterator { return primary_map.cbegin(); }
			auto cend() const-> const_iterator { return primary_map.cend(); }

			auto find(const key_type& key) -> iterator {
				auto primary_it = primary_map.find(key);
				if (primary_it != primary_map.end()) {
					return primary_it;
				}

				auto secondary_it = secondary_to_primary_map.find(key);
				if (secondary_it != secondary_to_primary_map.end()) {
					return primary_map.find(secondary_it->second);
				}

				return end();
			}

			auto find(const key_type& key) const -> const_iterator {
				auto primary_it = primary_map.find(key);
				if (primary_it != primary_map.end()) {
					return primary_it;
				}

				auto secondary_it = secondary_to_primary_map.find(key);
				if (secondary_it != secondary_to_primary_map.end()) {
					return primary_map.find(secondary_it->second);
				}

				return end();
			}

			auto empty() const -> bool {
				return primary_map.empty();
			}

			auto size() const -> std::size_t {
				return primary_map.size();
			}
		private:
			std::map<key_type, value_type> primary_map;
			std::unordered_map<key_type, key_type> secondary_to_primary_map;
		};

		// Basic wrappers for erasing the type of our store values

		struct value_wrapper_interface {
			virtual auto parse(const std::string& value) -> std::any = 0;
			virtual ~value_wrapper_interface() = default;

			// used for cases when the value is implicit (ie. pure flags) 
			bool expects_value = true;
		};

		template<typename type>
		struct value_wrapper : value_wrapper_interface {
			auto parse(const std::string& value) -> std::any override {
				return options_parser<type>::parse(value);
			}
		};

		using value_wrapper_ptr = std::shared_ptr<value_wrapper_interface>;
	} // namespace detail

	/**
	 * \brief Pack of parameters which were extracted from userland arguments.
	 */
	class parameters {
	public:
		/**
		 * \brief Checks whether the parameter pack contains a given key.
		 * \param name Key to check
		 * \return True if the pack contains the given key, false otherwise
		 */
		auto contains(const std::string& name) const -> bool {
			return m_values.contains(name);
		}

		/**
		 * \brief Retrieves a given key from the pack.
		 * \tparam type Type of the given key
		 * \param name Name of the given key
		 * \return Value of type \b type held under \b key.
		 */
		template<typename type = bool>
		auto get(const std::string& name) const -> type {
			return std::any_cast<type>(m_values.at(name));
		}
	private:
		std::unordered_map<std::string, std::any> m_values;

		friend class flag;
		friend class positional_argument;
	};

	/**
	 * \brief Context which is passed around when parsing the parameter tree.
	 */
	struct context {
		context(int argc, char** argv) : arguments(argv, argv + argc) {
			arguments[0] = std::filesystem::path(argv[0]).filename().string();
		}

		auto can_get_next_value() const -> bool {
			return argument_index + 1 <= arguments.size();
		}

		auto get_next_value() -> const std::string& {
			return arguments[argument_index++];
		}

		void begin_new_scope() {
			m_scoped_argument_index = argument_index;
		}

		/**
		 * \brief Creates a string representing all values from \a arguments [0] \a -
		 * \a arguments[\a scoped \a argument \a index] separated by a space (' ') char.
		 * \return String representation of our argument range.
		 */
		auto get_value_string() const -> std::string {
			std::ostringstream oss;

			for (std::size_t i = 0; i < m_scoped_argument_index && i < arguments.size(); ++i) {
				if (i > 0) {
					oss << ' ';
				}

				oss << arguments[i];
			}

			return oss.str();
		}

		std::vector<std::string> arguments; // userland arguments

		std::size_t argument_index = 1;            // local argument index used when parsing
		std::size_t positional_argument_index = 0; // index of our positional arguments

		// final output parameters, which are incrementally constructed when parsing
		parameters output_parameters;
	private:
		// helper index which is updated when a new scope (command/command group) is created
		// used for clearer help prints
		std::size_t m_scoped_argument_index = 0;
	};

	/**
	 * \brief Represents a simple flag (either --flag or -f). A flag can either implicitly represent
	 * a value (when it's declared with a bool type), or a flag which requires a succeeding value.
	 * When using a custom type a parametric::options_parser overload for the given type must be
	 * defined.
	 */
	class flag {
	public:
		flag() = default;
		flag(const std::string& long_name, const std::string& description, const std::string& short_name, const detail::value_wrapper_ptr& value_interface)
			: m_long_name(long_name), m_short_name(short_name), m_description(description), m_value_interface(value_interface) {}
	private:
		auto parse_next(context& context, const std::string& provided_flag_name, const std::string& provided_flag_name_no_dashes) -> int {
			// check dash-correctness
			// since we know our flag has at most two dashes, we don't have to do error checking here
			if (detail::is_flag(provided_flag_name)) {
				const bool is_verbose = detail::is_verbose_flag(provided_flag_name);

				if ((m_long_name == provided_flag_name_no_dashes) != is_verbose) {
					if (m_short_name.empty()) {
						// missing long alias
						std::cout << std::format("error: invalid flag verbosity '{}' (did you mean to use '--{}'?)\n", provided_flag_name, m_long_name);
					}
					else {
						// invalid short/long alias
						std::cout << std::format("error: invalid flag verbosity '{}' (did you mean to use '-{}' or '--{}'?)\n", provided_flag_name, m_short_name, m_long_name);
					}

					return ERROR_RESULT;
				}
			}
			else {
				// not a flag
				std::cout << std::format("error: invalid flag format '{}' (did you mean to use {}?)\n", provided_flag_name, get_label());
				return ERROR_RESULT;
			}

			// parse the final value
			if (m_value_interface->expects_value) {
				if (!context.can_get_next_value()) {
					std::cout << std::format("error: missing value for flag '{}'\n", provided_flag_name);
					return ERROR_RESULT;
				}

				const std::string value = context.get_next_value();

				try {
					context.output_parameters.m_values[m_long_name] = m_value_interface->parse(value);
				}
				catch (...) {
					std::cout << std::format("error: invalid format for flag '{}'\n", provided_flag_name);
					return ERROR_RESULT;
				}
			}
			else {
				// pure flag, mark it
				context.output_parameters.m_values[m_long_name] = true;
			}

			return 0;
		}

		auto get_label() const -> std::string {
			if (!m_short_name.empty()) {
				return std::format("[--{} | -{}]", m_long_name, m_short_name);
			}

			return std::format("[--{}]", m_long_name);
		}

		void use_default_if_unused(context& context) const {
			if (!m_default_value.has_value()) {
				return;
			}

			if (!context.output_parameters.contains(m_long_name)) {
				context.output_parameters.m_values[m_long_name] = m_default_value;
			}
		}
	private:
		// info
		std::string m_long_name;
		std::string m_short_name;
		std::string m_description;

		// parse function used when handling value conversions
		detail::value_wrapper_ptr m_value_interface;

		std::any m_default_value; // optional default value

		friend class command;
	};

	/**
	 * \brief Represents a simple positional argument. Positional arguments are \b mandatory and
	 * should always be specified in a given order. Each argument maps is guaranteed to map to a
	 * value in \b parameters. When using a custom type a parametric::options_parser overload for
	 * the given type must be defined.
	 */
	class positional_argument {
	public:
		positional_argument() = default;
		positional_argument(const std::string& name, const std::string& description, const detail::value_wrapper_ptr& value_interface)
			: m_name(name), m_description(description), m_value_interface(value_interface) {}
	private:
		auto parse_next(context& context) const -> int {
			const std::string value = context.arguments[context.argument_index - 1];

			try {
				context.output_parameters.m_values[m_name] = m_value_interface->parse(value);
			}
			catch (...) {
				std::cout << std::format("error: invalid format for positional argument '{}'\n", m_name);
				return ERROR_RESULT;
			}

			return 0;
		}
	private:
		// info
		std::string m_name;
		std::string m_description;

		// parse function used when handling value conversions
		detail::value_wrapper_ptr m_value_interface;

		friend class command;
	};

	class command_base {
	public:
		command_base() = default;
		command_base(const std::string& description) : m_description(description) {}

		virtual ~command_base() = default;

	protected:
		virtual void print_help(context& context) = 0;
		virtual auto parse_next(context& context) -> int = 0;
	protected:
		std::string m_description;

		friend class command_group;
	};

	/**
	 * \brief Represents a basic command (such as 'add' in 'git add'). Each command is the final unit
	 * of logic before userland code is executed. Once a command is run be the user the specified
	 * function is executed and the parsed parameters are provided as the parameter.
	 */
	class command : public command_base {
	public:
		using function = std::function<int(const parameters&)>;

		command() = default;
		command(const std::string& name, const std::string& description, const function& func)
			: command_base(description), m_name(name), m_function(func) {}

		/**
		 * \brief Appends a new flag to the command.
		 * \tparam type Type of the flag (defaults to bool, if any other type is specified the flag will
		 * also expect a succeeding value to be specified)
		 * \param long_name Long flag alias (--flag)
		 * \param description Description of the flag
		 * \param short_name Short flag alias (-f)
		 * \return Reference to the newly created flag.
		 */
		template<typename type = bool>
		auto add_flag(const std::string& long_name, const std::string& description, const std::string& short_name = "") -> flag& {
			auto value_interface = std::make_shared<detail::value_wrapper<type>>();

			flag new_flag(long_name, description, short_name, value_interface);

			// handle booleans as flags which don't require a value
			if constexpr (std::is_same_v<type, bool>) {
				value_interface->expects_value = false;
				new_flag.m_default_value = false;
			}

			if (!short_name.empty()) {
				return m_flags.insert(long_name, short_name, new_flag);
			}

			return m_flags.insert(long_name, new_flag);
		}

		/**
		 * \brief Appends a new flag to the command and assigns its default value.
		 * \tparam type Type of the flag (defaults to bool, if any other type is specified the flag will
		 * also expect a succeeding value to be specified)
		 * \param long_name Long flag alias (--flag)
		 * \param description Description of the flag
		 * \param short_name Short flag alias (-f)
		 * \param default_value Default value which will be used if the flag isn't mentioned by the user
		 * \return Reference to the newly created flag.
		 */
		template<typename type = bool>
		auto add_flag(const std::string& long_name, const std::string& description, const std::string& short_name, const type& default_value) -> flag& {
			flag& flag = add_flag<type>(long_name, description, short_name);

			// assign the expected value
			flag.m_default_value = default_value;

			return flag;
		}

		/**
		 * \brief Appends a new positional argument. Positional arguments need to be specified before any
		 * flags, but after commands. \b Positional \b arguments \b are \b mandatory and may not be ignored
		 * by the user. The order of individual positional arguments is the same as the order in which the
		 * add_positional_argument was called.
		 * \tparam type Type the argument should be parsed as
		 * \param name Name of the positional argument
		 * \param description Description of the positional argument
		 */
		template<typename type>
		void add_positional_argument(const std::string& name, const std::string& description) {
			m_arguments.emplace_back(name, description, std::make_shared<detail::value_wrapper<type>>());
		}
	private:
		auto parse_next(context& context) -> int override {
			context.begin_new_scope(); // push a new scope

			// parse all remaining options
			while (context.can_get_next_value()) {
				std::string value = context.get_next_value();

				// if the current key is a help flag
				if (detail::is_help_flag(value)) {
					print_help(context);
					return ERROR_RESULT;
				}

				// prefer parsing positional arguments first
				if (context.positional_argument_index < m_arguments.size()) {
					if (m_arguments[context.positional_argument_index++].parse_next(context) != ERROR_RESULT) {
						continue;
					}

					return ERROR_RESULT; // handled by the parse_next call
				}

				// check against our flag list
				std::string value_no_dashes;

				if (!detail::remove_leading_dashes(value, value_no_dashes)) {
					// our string has too many dashes, exit
					std::cout << std::format("error: unknown argument '{}'", value);
					print_help(context);
					return ERROR_RESULT;
				}

				// locate the flag
				const auto flag_it = m_flags.find(value_no_dashes);
				if (flag_it != m_flags.end()) {
					if (flag_it->second.parse_next(context, value, value_no_dashes) != ERROR_RESULT) {
						continue;
					}

					return ERROR_RESULT; // handled by the parse_next call
				}

				std::cout << std::format("error: unknown argument '{}'\n", value);
				print_help(context);
				return ERROR_RESULT;
			}

			// require all positional arguments to be specified
			if (context.positional_argument_index != m_arguments.size()) {
				std::cout << std::format("error: missing positional argument/s for command '{}'\n", m_name);
				print_help(context);
				return ERROR_RESULT;
			}

			// at this point, we've parse_next all of our arguments
			// check against default arguments
			for (const auto& [name, flag] : m_flags) {
				flag.use_default_if_unused(context);
			}

			// run our function
			return m_function(context.output_parameters);
		}

		void print_help(context& context) override {
			const std::string value_string = context.get_value_string();
			std::cout << std::format("usage: {} ", value_string);

			// print a list of available arguments/flags
			std::vector<std::string> labels;
			labels.reserve(m_arguments.size() + m_flags.size());

			for (const auto& arg : m_arguments) {
				labels.emplace_back(std::format("<{}> ", arg.m_name));
			}

			for (const auto& [name, flag] : m_flags) {
				labels.emplace_back(flag.get_label() + " ");
			}

			detail::print_aligned_strings(labels, value_string.size() + 8, PARAMETRIC_TABLE_WIDTH);
			std::cout << "\n";

			// print a description of available arguments/flags
			if (!m_arguments.empty() || !m_flags.empty()) {
				std::cout << '\n';
			}

			// find the longest argument/flag name
			std::size_t longest_name = 0;

			for (const auto& arg : m_arguments) {
				longest_name = std::max(arg.m_name.size(), longest_name);
			}

			for (const auto& [name, flag] : m_flags) {
				longest_name = std::max(name.size(), longest_name);
			}

			// handle arguments
			for (const auto& arg : m_arguments) {
				std::cout << std::format("   {:<{}s}{}\n", arg.m_name, longest_name + 3, arg.m_description);
			}

			if (!m_flags.empty() && !m_arguments.empty()) {
				std::cout << '\n';
			}

			// handle flags
			for (const auto& [name, flag] : m_flags) {
				std::cout << std::format("   {:<{}s}{}\n", name, longest_name + 3, flag.m_description);
			}
		}
	private:
		std::string m_name;  // name of the command
		function m_function; // function to invoke when the command is successfully executed

		detail::dual_map<std::string, flag> m_flags;  // command flags
		std::vector<positional_argument> m_arguments; // list of positional arguments, stored in order of appearances
	};

	/**
	 * \brief Grouping of commands, can either hold commands or other command groups. A command
	 * group may not be executed on its own and serves only as a wrapper for more commands or
	 * command groups.
	 */
	class command_group : public command_base {
	public:
		command_group() = default;
		command_group(const std::string& description) : command_base(description) {}

		/**
		 * \brief Appends a new child command group.
		 * \param name Name of the new command group
		 * \param description Description of the new command group
		 * \return Reference to the new command group.
		 */
		command_group& add_command_group(const std::string& name, const std::string& description) {
			const auto group = std::make_shared<command_group>(description);
			m_commands[name] = group;
			return *group;
		}

		/**
		 * \brief Appends a new child command.
		 * \param name Name of the new command
		 * \param description Description of the new command
		 * \param func Function which will be executed if the command is chosen by the user
		 * \return Reference to the new command.
		 */
		command& add_command(const std::string& name, const std::string& description, const command::function& func) {
			const auto cmd = std::make_shared<command>(name, description, func);
			m_commands[name] = cmd;
			return *cmd;
		}
	protected:
		auto parse_next(context& context) -> int override {
			context.begin_new_scope(); // push a new scope

			// verify that we have a command key
			if (!context.can_get_next_value()) {
				std::cout << "error: missing command\n";
				print_help(context);
				return ERROR_RESULT;
			}

			std::string value = context.get_next_value();

			// if the current key is a help flag
			if (detail::is_help_flag(value)) {
				print_help(context);
				return ERROR_RESULT;
			}

			// look for a command/command group
			const auto command_it = m_commands.find(value);
			if (command_it != m_commands.end()) {
				return command_it->second->parse_next(context);
			}

			std::cout << std::format("error: unknown command '{}'\n", value);
			print_help(context);
			return ERROR_RESULT;
		}

		void print_help(context& context) override {
			const std::string value_string = context.get_value_string();
			std::cout << std::format("usage: {} ", value_string);

			// print a list of available subcommands
			std::vector<std::string> labels;
			labels.reserve(m_commands.size());

			for (const auto& [name, subcommand] : m_commands) {
				labels.emplace_back(std::format("[{}] ", name));
			}

			detail::print_aligned_strings(labels, value_string.size() + 8, PARAMETRIC_TABLE_WIDTH);
			std::cout << "\n\n";

			// find the longest subcommand name
			std::size_t longest_name = 0;

			for (const auto& [name, subcommand] : m_commands) {
				longest_name = std::max(name.size(), longest_name);
			}

			// print all subcommands
			//    name<--padding-->description
			for (const auto& [name, subcommand] : m_commands) {
				std::cout << std::format("   {:<{}s}{}\n", name, longest_name + 3, subcommand->m_description);
			}
		}
	protected:
		std::map<std::string, std::shared_ptr<command_base>> m_commands; // child commands/command groups
	};

	/**
	 * \brief Virtual representation of a program and it's arguments/runtime.
	 */
	class program : public command_group {
	public:
		/**
		 * \brief Parses the specified program parameters (\b argc, \b argv), and calls the respective function of
		 * the chosen command.
		 * \param argc Count of provided arguments
		 * \param argv Argument values
		 * \return Return code returned by the function of the chosen program. If the help menu is show to the user
		 * the program exits with '0'.
		 */
		auto parse(int argc, char** argv) -> int {
			context ctx(argc, argv);
			int result = parse_next(ctx);

			// since we don't actually want to return ERROR_RESULT, we override it to '0'
			if (result == ERROR_RESULT) {
				result = 0;
			}

			return result;
		}
	};
} // namespace parametric
