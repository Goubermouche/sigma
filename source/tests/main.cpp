#include <utility/parametric/parametric.h>
#include <utility/filesystem/file.h>
#include <utility/string_helper.h>
#include <utility/diagnostics.h>
#include <utility/shell.h>

using namespace utility::types;

#define COMPILER_STDOUT "compiler_STDOUT.txt"
#define COMPILER_STDERR "compiler_STDERR.txt"

#define CLANG_STDOUT "clang_STDOUT.txt"
#define CLANG_STDERR "clang_STDERR.txt"

#define APP_STDOUT "app_STDOUT.txt"
#define APP_STDERR "app_STDERR.txt"

#ifdef SYSTEM_WINDOWS
#define OBJECT_FILE "test.obj"
#define EXECUTABLE_FILE "test.exe"
#define SYSTEM_STR "windows"
#define EXECUTABLE_OPT ""
#else
#define OBJECT_FILE "test.o"
#define EXECUTABLE_FILE "test"
#define SYSTEM_STR "linux"
#define EXECUTABLE_OPT "./"
#endif


auto read_or_throw(const filepath& path) -> std::string {
	const auto result = utility::fs::file<std::string>::load(path);
	if (result.has_error()) {
		throw std::runtime_error(result.get_error().get_message());
	}

	return result.get_value();
}

void print_error_block(const std::vector<std::string>& headers, std::vector<std::string> messages) {
	ASSERT(headers.size() == messages.size(), "incompatible block layout");

	for(u64 i = 0; i < headers.size(); ++i) {
		utility::console::print("{:-<60}\n", headers[i]);

		if (messages[i].empty()) {
			continue;
		}

		messages[i].pop_back(); // remove trailing newlines
		utility::console::print("  {}\n", messages[i]);

		if(i + 1 >= headers.size()) {
			utility::console::print("{}\n", std::string(60, '-'));
		}
	}
}

auto get_expected_path(const filepath& path) -> filepath {
	return path.get_parent_path() / (path.get_filename_no_ext().to_string() + "_expected.txt");
}

auto get_pretty_path(const filepath& path) -> filepath {
	return path.get_parent_path().get_filename() / path.get_filename_no_ext();
}

auto compile_file(const filepath& path, const filepath& compiler_path) -> bool {
	const std::string compilation_command = std::format("{} compile {} -e {} --system {} > {} 2> {}", compiler_path, path, OBJECT_FILE, SYSTEM_STR, COMPILER_STDOUT, COMPILER_STDERR);
	const std::string link_command = std::format("clang {} -o {} ", OBJECT_FILE, EXECUTABLE_FILE);

	// compile the source file
	if(utility::shell::execute(compilation_command) != 0) {
		utility::console::printerr("{:<40} ERROR (compile)\n", get_pretty_path(path).to_string());

		const std::string stdout_str = read_or_throw(COMPILER_STDOUT);
		const std::string stderr_str = read_or_throw(COMPILER_STDERR);

		print_error_block({ "STDOUT", "STDERR" }, { stdout_str , stderr_str });

		return true;
	}

	// link the generated object file
	if(utility::shell::execute(link_command) != 0) {
		utility::console::printerr("{:<40} ERROR (link)\n", get_pretty_path(path).to_string());

		const std::string stdout_str = read_or_throw(CLANG_STDOUT);
		const std::string stderr_str = read_or_throw(CLANG_STDERR);

		print_error_block({ "STDOUT", "STDERR" }, { stdout_str , stderr_str });

		return true;
	}

	return false;
}

auto run_executable(const filepath& path) -> i32 {
	const std::string command = std::format("{}{} > {} 2> {}", EXECUTABLE_OPT, path, APP_STDOUT, APP_STDERR);
	return utility::shell::execute(command);
}

bool run_test(const filepath& path, const filepath& compiler_path) {
	const filepath pretty_path = path.get_parent_path().get_filename() / path.get_filename_no_ext();

	if(compile_file(path, compiler_path)) {
		return true;
	}

	if(i32 run_result = run_executable(EXECUTABLE_FILE)) {
		utility::console::printerr("{:<40} ERROR (run - {})\n", path.to_string(), run_result);

		const std::string app_stdout_str = read_or_throw(APP_STDOUT);
		const std::string app_stderr_str = read_or_throw(APP_STDERR);
		const std::string compiler_stdout_str = read_or_throw(COMPILER_STDOUT);
		const std::string compiler_stderr_str = read_or_throw(COMPILER_STDERR);

		print_error_block(
			{ "STDOUT", "STDERR", "COMPILER_STDOUT", "COMPILER_STDERR" },
			{
				utility::detail::escape_string(app_stdout_str),
				utility::detail::escape_string(app_stderr_str),
				compiler_stdout_str,
				compiler_stderr_str
			}
		);

		return true;
	}

	// executable returned 0
	const std::string app_stdout_str = read_or_throw(APP_STDOUT);
	const std::string expected_str = read_or_throw(get_expected_path(path));

	if(app_stdout_str != expected_str) {
		utility::console::printerr("{:<40} ERROR (unexpected result)\n", pretty_path.to_string());

		const std::string app_stderr_str = read_or_throw(APP_STDERR);
		const std::string compiler_stdout_str = read_or_throw(COMPILER_STDOUT);
		const std::string compiler_stderr_str = read_or_throw(COMPILER_STDERR);

		print_error_block(
			{ "STDOUT", "STDERR", "COMPILER_STDOUT", "COMPILER_STDERR" },
			{
				utility::detail::escape_string(app_stdout_str),
				utility::detail::escape_string(app_stderr_str),
				compiler_stdout_str,
				compiler_stderr_str
			}
		);


		return true;
	}

	utility::console::print("{:<40} OK\n", pretty_path.to_string());
	return false;
}

i32 run_all_tests(const parametric::parameters& params) {
	const filepath test_directory = params.get<std::string>("directory");
	const filepath compiler_path = params.get<std::string>("compiler");

	std::queue<filepath> paths({ test_directory.get_canonical_path() });
	bool encountered_error = false;

	// initialize our files
	try {
		utility::fs::create(COMPILER_STDOUT);
		utility::fs::create(COMPILER_STDERR);
		utility::fs::create(CLANG_STDOUT);
		utility::fs::create(CLANG_STDERR);
		utility::fs::create(APP_STDOUT);
		utility::fs::create(APP_STDERR);
		utility::fs::create(OBJECT_FILE);
		utility::fs::create(EXECUTABLE_FILE);
	}
	catch (const std::exception& exception) {
		utility::console::printerr("error: {}\n", exception.what());
		encountered_error = true;
	}

	if(encountered_error) {
		return 1;
	}

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
		utility::fs::remove(COMPILER_STDOUT);
		utility::fs::remove(COMPILER_STDERR);
		utility::fs::remove(CLANG_STDOUT);
		utility::fs::remove(CLANG_STDERR);
		utility::fs::remove(APP_STDOUT);
		utility::fs::remove(APP_STDERR);
		utility::fs::remove(OBJECT_FILE);
		utility::fs::remove(EXECUTABLE_FILE);
	}
	catch (const std::exception& exception) {
		utility::console::printerr("error: {}\n", exception.what());
		encountered_error = true;
	}

	return encountered_error;
}

i32 main(i32 argc, char* argv[]) {
	parametric::program program;

	auto& run_all = program.add_command("run", "run all tests in the specified directory", run_all_tests);
	run_all.add_positional_argument<std::string>("directory", "test directory");
	run_all.add_positional_argument<std::string>("compiler", "compiler executable path");

	return program.parse(argc, argv);
}
