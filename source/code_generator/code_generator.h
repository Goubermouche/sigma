#pragma once
#include "intermediate_code/intermediate_code.h"

using namespace utility::types;

namespace sigma {
	//constexpr u32 g_pe_signature = 0x00004550;  // "PE\0\0"

	//struct import_by_name {
	//	import_by_name(
	//		const std::string& name
	//	);

	//	ptr<IMAGE_IMPORT_BY_NAME> get_value() const;
	//private:
	//	ptr<IMAGE_IMPORT_BY_NAME> m_value;
	//};

	//template<typename type>
	//std::vector<unsigned char> serialize(const type& value) {
	//	std::vector<unsigned char> vec(sizeof(value));
	//	std::memcpy(vec.data(), &value, sizeof(value));
	//	return vec;
	//}

	//class import_table {
	//public:
	//	void add_import(
	//		const std::string& dll_name,
	//		const std::vector<std::string>& imports,
	//		u64 start_RVA
	//	);

	//	const std::vector<unsigned char>& get_bytecode() const;
	//private:

	//	std::vector<unsigned char> m_bytecode;
	//};

	//class pe_builder {
	//public:
	//	pe_builder();

	//	void add_section(
	//		const std::string& name,
	//		std::vector<unsigned char> data,
	//		windows::section_header_characteristics characteristics
	//	);

	//	u64 get_current_RVA() const;

	//	void emit_to_file(
	//		const filepath& path
	//	);
	//private:
	//	windows::dos_header m_dos_header = {};
	//	windows::file_header m_file_header = {};
	//	windows::optional_header_64_bit m_optional_header = {};

	//	std::vector<windows::section_header> m_sections;
	//	std::vector<std::vector<unsigned char>> m_section_data;
	//	u64 m_current_RVA;
	//};

	//class context {
	//public:
	//	utility::outcome::result<void> compile(
	//		const filepath& path
	//	);
	//private:
	//};

	//class target {
	//	
	//};



	//struct ret {};

	//using instruction = std::variant<mov, ret>;

	//class backend {
	//public:
	//	// using op_code_table = std::unordered_map<register_id, std::unordered_map<register_id, std::vector<utility::byte>>>;

	//	virtual ~backend() {}
	//	virtual std::vector<utility::byte> generate_code(
	//		const instruction& instruction
	//	) = 0;
	//};

	//class amd_backend : public backend {
	//public:
	//	std::vector<utility::byte> generate_code(
	//		const instruction& instruction
	//	) override;

	//	std::vector<utility::byte> operator()(const mov& mov) const;
	//	std::vector<utility::byte> operator()(const ret& ret) const;
	//private:
	//};

	//class code_generator {
	//public:
	//	code_generator(
	//		const target& target
	//	);

	//	void add_instruction(
	//		const instruction& instruction
	//	);

	//	std::vector<utility::byte> get_code() const;
	//private:
	//	std::vector<utility::byte> m_code;
	//	std::unique_ptr<backend> m_backend;
	//};

	class code_generator {
	public:
		code_generator(const ir::builder& builder);

		void allocate_registers();
	private:
		ir::builder m_builder;
	};
}
