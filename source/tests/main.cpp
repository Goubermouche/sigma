#include <utility/parametric/parametric.h>
#include <utility/filesystem/file.h>
#include <utility/shell.h>

using namespace utility::types;

#define STDOUT_FILE "STDOUT.txt"
#define STDERR_FILE "STDERR.txt"

bool run_test(const filepath& path, const filepath& compiler_path) {
	const std::string command = std::format("{} compile {} -e none > {} 2> {}", compiler_path, path, STDOUT_FILE, STDERR_FILE);
	const filepath& pretty_path = path.get_parent_path().get_filename() / path.get_filename();

	const i32 return_code = utility::shell::execute(command);

	if(return_code == 0) {
		utility::console::print("{:<40} OK\n", pretty_path.to_string());
		return false;
	}

	utility::console::printerr("{:<40} ERROR\n", pretty_path.to_string());

	const auto file_result = utility::fs::file<std::string>::load(STDERR_FILE);
	if(file_result.has_error()) {
		throw std::runtime_error(std::format("cannot open file {}", STDERR_FILE).c_str());
	}

	utility::console::printerr("'{}'\n", file_result.get_value());

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

				encountered_error |= run_test(path, compiler_path);
			});
		}
	} catch(const std::exception& exception) {
		utility::console::printerr("error: {}\n", exception.what());
		encountered_error = true;
	}

	// cleanup
	try {
		utility::fs::remove(STDOUT_FILE);
		utility::fs::remove(STDERR_FILE);
	} catch(const std::exception& exception) {
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
