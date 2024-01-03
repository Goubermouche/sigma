#include <utility/diagnostics.h>
#include <utility/filesystem/file.h>
#include <utility/string_helper.h>

// Runs all tests in sigma/tests and checks against a database of expected results. Should only
// be invoked via the run_tests.bat file.

//#define WIN_COMPILER_PATH utility::fs::get_canonical_path("../../../../output/compiler/bin/Debug/compiler.exe").string()
//#define ERR_FILE_PATH "err.txt"
//
using namespace utility::types;
//
//auto compile_file(const filepath& path) -> utility::result<bool> {
//	const std::string command = WIN_COMPILER_PATH + " " + utility::fs::get_canonical_path(path).string() + " NO_EMIT" + " 1> err.txt";
//	const bool has_error = system(command.c_str());
//
//	if(has_error) {
//		TRY(std::string message, utility::file::read_text_file(ERR_FILE_PATH));
//
//		// message = utility::detail::remove_first_line(message);
//		// message.erase(message.size() - 1); // remove the trailing newline
//		// utility::console::println("\033[31m'{}'\033[0m", message);
//	}
//
//	return has_error;
//}

//i32 main() {
//	std::queue<filepath> paths({ utility::fs::get_canonical_path("../../../../tests") });
//
//	std::cout << paths.front() << '\n';
//
//	utility::console::println("running tests...");
//	utility::console::println("{}", std::string(32, '-'));
//
//	while(!paths.empty()) {
//		const filepath current = paths.front();
//		paths.pop();
//
//		utility::directory::for_all_directory_items(current, [&](const filepath& path) {
//			if(utility::fs::is_directory(path)) {
//				paths.push(path);
//				return;
//			}
//
//			static const char* result_codes[] = {
//				"OK",
//				"ERROR",
//			};
//
//			const auto result = compile_file(path);
//
//			if(result.has_error()) {
//				utility::console::println(result.get_error_message());
//				return;
//			}
//
//			utility::console::println("{:<30}{}", path.stem().string(), result_codes[result.get_value()]);
//		});
//	}
//
//	utility::fs::remove(ERR_FILE_PATH);
//	return 0;
//}

i32 main(i32 argc, char* argv[]) {
	// 
}