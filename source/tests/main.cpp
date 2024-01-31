#include <utility/parametric/parametric.h>
#include <utility/filesystem/file.h>
#include <utility/string_helper.h>
#include <utility/shell.h>

using namespace utility::types;

#define STDOUT_FILE "STDOUT.txt"
#define STDERR_FILE "STDERR.txt"

auto get_object_path(const filepath& path) -> filepath {
#ifdef SYSTEM_WINDOWS
	const char* format = "a.obj";
#else
	const char* format = "a.o";
#endif

	return path.get_parent_path() / format;
}

auto get_executable_path(const filepath& path) -> filepath {
#ifdef SYSTEM_WINDOWS
	const char* format = "a.exe";
#else
	const char* format = "a";
#endif

	return path.get_parent_path() / format;
}

auto get_expected_path(const filepath& path) -> filepath {
	return path.get_parent_path() / (path.get_filename_no_ext().to_string() + "_expected.txt");
}

auto compile_file(const filepath& path, const filepath& compiler_path) -> bool {
	const filepath pretty_path = path.get_parent_path().get_filename() / path.get_filename_no_ext();

	const std::string compilation_command = std::format("{} compile {} > {} 2> {}", compiler_path, path, STDOUT_FILE, STDERR_FILE);
	const std::string link_command = std::format("clang -o {} {}", get_executable_path(path), get_object_path(path));

	if(utility::shell::execute(compilation_command) != 0) {
		utility::console::printerr("{:<40} ERROR (compile)\n", pretty_path.to_string());
		return false;
	}

	if(utility::shell::execute(link_command) != 0) {
		utility::console::printerr("{:<40} ERROR (link)\n", pretty_path.to_string());
		return false;
	}

	return true;
}

auto run_executable(const filepath& path) -> bool {
	const std::string command = std::format("{} > {} 2> {}", get_executable_path(path).to_string(), STDOUT_FILE, STDERR_FILE);
	return !utility::shell::execute(command);
}

void cleanup(const filepath& path) {
	const filepath object_path = get_object_path(path);
	const filepath executable_path = get_executable_path(path);

	if (object_path.exists()) {
		utility::fs::remove(object_path);
	}

	if (executable_path.exists()) {
		utility::fs::remove(executable_path);
	}
}

auto read_or_throw(const filepath& path) -> std::string {
	const auto result = utility::fs::file<std::string>::load(path);
	if (result.has_error()) {
		throw std::runtime_error(result.get_error().get_message());
	}

	return result.get_value();
}

bool run_test(const filepath& path, const filepath& compiler_path) {
	const filepath pretty_path = path.get_parent_path().get_filename() / path.get_filename_no_ext();

	if (compile_file(path, compiler_path)) {
		// OK, no errors compiling, compare results
		if (run_executable(path)) {
			// executable returned 0
			const std::string stdout_str = read_or_throw(STDOUT_FILE);
			const std::string expected_str = read_or_throw(get_expected_path(path));

			if (stdout_str == expected_str) {
				utility::console::print("{:<40} OK\n", pretty_path.to_string());
			}
			else {
				utility::console::printerr("{:<40} ERROR (C, E)\n", pretty_path.to_string());
				utility::console::printerr("{}\n", std::string(53, '-'));
				utility::console::printerr("unexpected STDOUT contents - got:\n");
				utility::console::printerr("'{}'\n", utility::detail::escape_string(stdout_str));
				utility::console::printerr("expected string is:\n");
				utility::console::printerr("'{}'\n", utility::detail::escape_string(expected_str));
				utility::console::printerr("{}\n", std::string(53, '-'));
			}
		}
	}
	else {
		// ERROR when compiling
		utility::console::printerr("{:<40} ERROR\n", pretty_path.to_string());

		const std::string stdout_str = read_or_throw(STDOUT_FILE);
		const std::string stderr_str = read_or_throw(STDERR_FILE);

		utility::console::printerr("'{}'\n", stderr_str);
	}

	cleanup(path);

	// const std::string command = std::format("{} compile {} -e none > {} 2> {}", compiler_path, path, STDOUT_FILE, STDERR_FILE);
	// const filepath& pretty_path = path.get_parent_path().get_filename() / path.get_filename();
	// 
	// const i32 return_code = utility::shell::execute(command);
	// 
	// if(return_code == 0) {
	// 	utility::console::print("{:<40} OK\n", pretty_path.to_string());
	// 	return false;
	// }
	// 
	// utility::console::printerr("{:<40} ERROR\n", pretty_path.to_string());
	// 
	// const auto file_result = utility::fs::file<std::string>::load(STDERR_FILE);
	// if(file_result.has_error()) {
	// 	throw std::runtime_error(std::format("cannot open file {}", STDERR_FILE).c_str());
	// }
	// 
	// utility::console::printerr("'{}'\n", file_result.get_value());

	return true;
}

i32 run_all_tests(const parametric::parameters& params) {
	const filepath test_directory = params.get<std::string>("directory");
	const filepath compiler_path = params.get<std::string>("compiler");

	std::queue<filepath> paths({ test_directory.get_canonical_path() });
	bool encountered_error = false;

	// run our tests
	try {
		while (!paths.empty()) {
			const filepath current = paths.front();
			paths.pop();

			utility::fs::directory::for_all_directory_items(current, [&](const filepath& path) {
				if (path.is_directory()) {
					paths.push(path);
					return;
				}

				if (path.get_extension() == ".s") {
					// only compile .s files
					encountered_error |= run_test(path, compiler_path);
				}
				});
		}
	}
	catch (const std::exception& exception) {
		utility::console::printerr("error: {}\n", exception.what());
		encountered_error = true;
	}

	// cleanup
	try {
		utility::fs::remove(STDOUT_FILE);
		utility::fs::remove(STDERR_FILE);
	}
	catch (const std::exception& exception) {
		utility::console::printerr("error: {}\n", exception.what());
		encountered_error = true;
	}

	return encountered_error ? 1 : 0;
}

i32 main(i32 argc, char* argv[]) {
	parametric::program program;

	auto& run_all = program.add_command("run", "run all tests in the specified directory", run_all_tests);
	run_all.add_positional_argument<std::string>("directory", "test directory");
	run_all.add_positional_argument<std::string>("compiler", "compiler executable path");

	return program.parse(argc, argv);
}
