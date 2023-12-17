#include <utility/diagnostics.h>
#include <utility/filesystem/file.h>
#include <utility/containers/stack.h>

// Runs all tests in sigma/tests and checks against a database of expected results. Should only
// be invoked via the run_tests.bat file.

#define WIN_COMPILER_PATH utility::fs::get_canonical_path("./output/compiler/bin/Debug/compiler.exe").string()
#define ERR_FILE_PATH "err.txt"

using namespace utility::types;

auto compile_file(const filepath& path) -> bool {
	const std::string command = WIN_COMPILER_PATH + " " + utility::fs::get_canonical_path(path).string() + " 1> err.txt";
	const bool error = system(command.c_str());

	if(error) {
		std::string message = utility::file::read_text_file(ERR_FILE_PATH);
		message.erase(message.size() - 1); // remove the trailing newline
		utility::console::println("\033[31m'{}'\033[0m", message);
	}

	return error;
}

i32 main() {
	std::queue<filepath> paths({ utility::fs::get_canonical_path("./tests") });

	utility::console::println("running tests...");
	utility::console::println("{}", std::string(32, '-'));

	while(!paths.empty()) {
		const filepath current = paths.front();
		paths.pop();

		utility::directory::for_all_directory_items(current, [&](const filepath& path) {
			if(utility::fs::is_directory(path)) {
				paths.push(path);
				return;
			}

			static const char* result_codes[] = {
				"OK",
				"ERROR",
			};

			const bool error = compile_file(path);
			utility::console::println("{:<30}{}", path.stem().string(), result_codes[error]);
		});
	}

	utility::fs::remove(ERR_FILE_PATH);
	return 0;
}
