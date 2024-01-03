#include <utility/filesystem/file.h>
#include <utility/parametric/parametric.h>
#include <utility/shell.h>

using namespace utility::types;

#define STOUD_FILE "STDOUT.txt"
#define STERR_FILE "STDERR.txt"

auto get_compiler_path() -> std::string {
#ifdef SYSTEM_WINDOWS
	return utility::fs::get_canonical_path("../../output/compiler/bin/Debug/compiler.exe").string();
#elif defined SYSTEM_LINUX
	return utility::fs::get_canonical_path("../../output/compiler/bin/Debug/compiler").string();
#else
	PANIC("unhandled system path");
	return std::string();
#endif
}

bool test_file(const filepath& path) {
	const std::string command = std::format("{} compile {} -e none > {} 2> {}", get_compiler_path(), path.string(), STOUD_FILE, STERR_FILE);
	const i32 return_code = utility::shell::execute(command);

	if(return_code == 0) {
		utility::console::print("{:<20} OK\n", path.filename().string());
		return false;
	}

	utility::console::printerr("{:<20} ERROR\n", path.filename().string());

	const auto file_result = utility::file::read_text_file(STERR_FILE);
	if(file_result.has_error()) {
		throw std::exception(std::format("cannot open file {}", STERR_FILE).c_str());
	}

	utility::console::printerr("'{}'\n", file_result.get_value());

	return true;
}

i32 run_all_tests(const parametric::parameters& params) {
	const auto test_directory = params.get<std::string>("directory");
	std::queue<filepath> paths({ utility::fs::get_canonical_path(test_directory) });
	bool encountered_error = false;

	// run our tests
	try {
		while (!paths.empty()) {
			const filepath current = paths.front();
			paths.pop();

			utility::directory::for_all_directory_items(current, [&](const filepath& path) {
				if (utility::fs::is_directory(path)) {
					paths.push(path);
					return;
				}

				encountered_error |= test_file(path);
			});
		}
	} catch(const std::exception& exception) {
		utility::console::printerr("error: {}\n", exception.what());
		encountered_error = true;
	}

	// cleanup
	try {
		utility::fs::remove(STOUD_FILE);
		utility::fs::remove(STERR_FILE);
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

	return program.parse(argc, argv);
}
