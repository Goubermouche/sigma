#pragma once
#include "intermediate_representation/node_hierarchy/function.h"

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

	struct module_output {
		void add_section(const std::string& name, module_section::module_section_flags flags, comdat::comdat_type comdat);

		std::vector<module_section> sections;
		handle<symbol> chkstk_extern = nullptr;
	};
} // namespace sigma::ir
