#pragma once
#include "intermediate_representation/codegen/codegen_target.h"

// The entire IR system is based off of an implementation in Cuik's Tilde backend
// (https://github.com/RealNeGate/Cuik/tree/master/tb)

// Root code generation file, contains the main module structure. Each module
// contains a list of functions, and every function contains an allocator through
// which it allocates and stores nodes, which represent the operation tree of the
// given function.
// Whilst it is possible to use the module class and its member to modify the node
// tree, I'd recommend you to use the builder class (see builder.h), which provides
// a more unified interface to the respective tasks.
// The module class itself should only be used as a storage medium for the node
// tree and for various final operations on it (compilation, emission of object
// files, etc.).

namespace sigma::ir {
	struct comdat {
		enum comdat_type {
			NONE,
			MATCH_ANY
		};

		u32 relocation_count;
		comdat_type ty;
	};

	struct module_section {
		enum module_section_flags {
			NONE = 0,
			WRITE = 1,
			EXEC = 2,
			TLS = 4,
		};

		std::string name;

		u32 export_flags;
		u32 name_position;
		module_section_flags flags;

		u16 section_index;
		comdat com;

		u32 raw_data_pos;
		u32 total_size;
		u32 relocation_count;
		u32 relocation_position;

		std::vector<handle<global>> globals;
		std::vector<handle<compiled_function>> functions;
	};

	FLAG_ENUM(module_section::module_section_flags);

	// TODO: implement a more performant, aligned memory allocator

	/**
	 * \brief Basic builder which serves as a facade around the process of
	 * constructing proper IR.
	 */
	class module {
	public:
		module(target target);

		void compile() const;
		auto generate_object_file() -> utility::object_file;

		auto create_external(const std::string& name, linkage linkage) -> handle<external>;
		auto create_function(const function_signature& function_sig, linkage linkage) -> handle<function>;

		auto create_global(const std::string& name, linkage linkage) -> handle<global>;
		auto create_string(handle<function> parent_function, const std::string& value) -> handle<node>;

		[[nodiscard]] auto get_target() const -> target;
	protected:
		auto get_sections() -> std::vector<module_section>&;
		auto get_sections() const -> const std::vector<module_section>&;

		void create_section(
			const std::string& name, 
			module_section::module_section_flags flags, 
			comdat::comdat_type comdat
		);

		auto generate_externals() -> std::vector<handle<external>>;

		static constexpr u8 get_text_section()  { return 0; }
		static constexpr u8 get_data_section()  { return 1; }
		static constexpr u8 get_rdata_section() { return 2; }
		static constexpr u8 get_tls_section()   { return 3; }

		// rough memory layout:
		//   module:
		//   +---------------------------------------------------------+
		//   | functions + symbols + globals + externals + export info |
		//   +---------------------------------------------------------+
		//   
		//   functions: 
		//   +---------------------------------------+
		//   | nodes + users + instructions + values |
		//   +---------------------------------------+

		utility::block_allocator m_allocator;
		codegen_target m_codegen;

		std::vector<handle<function>> m_functions;
		std::vector<handle<symbol>> m_symbols;
		std::vector<handle<global>> m_globals;

		std::vector<module_section> m_sections;
		handle<symbol> m_chkstk_extern;

		friend class coff_file_emitter;
		friend class elf_file_emitter;
	};
} // namespace sigma::ir
