#pragma once
#include<utility/types.h>

// Basic registers for the x64 platform, we just store the highest bit registers 
// and then reinterpret them as lower bit variants.

namespace ir::cg::x64 {
	using namespace utility::types;

	enum data_type {
		none = 0,

		byte,   word,   dword,  qword,
		pbyte,  pword,  pdword, pqword,
		sse_ss, sse_sd, sse_ps, sse_pd,

		xmmword
	};

	enum class register_class : u8 {
		gpr,
		xmm,
		first_gpr = 0,
		first_xmm = 16
	};

	enum gpr : u8 {
		rax, rcx, rdx, rbx, rsp, rbp, rsi, rdi,
		r8,  r9,  r10, r11, r12, r13, r14, r15,
	};

	enum xmm : u8 {
		xmm0, xmm1, xmm2,  xmm3,  xmm4,  xmm5,  xmm6,  xmm7,
		xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15
	};

	inline std::string get_register_name(reg reg, i32 dt) {
		static const char* s_gpr_names[4][16] = {
			{ "al",  "cl",  "dl",  "bl",  "spl", "bpl", "sil", "dil", "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b" },
			{ "ax",  "cx",  "dx",  "bx",  "sp",  "bp",  "si",  "di",  "r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w" },
			{ "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi", "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d" },
			{ "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi", "r8",  "r9",  "r10",  "r11",  "r12",  "r13",  "r14",  "r15"  }
		};

		if (dt >= byte && dt <= qword) {
			return s_gpr_names[dt - byte][reg.get_id()];
		}

		if (dt >= sse_ss && dt <= sse_pd) {
			static const char* s_xmm_names[] = {
				"xmm0", "xmm1", "xmm2",  "xmm3",  "xmm4",  "xmm5",  "xmm6",  "xmm7",
				"xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
			};

			return s_xmm_names[reg.get_id()];
		}

		return "??";
	}

	inline std::string get_type_name(i32 dt) {
		switch (dt) {
			case byte:   return "byte";
			case word:   return "word";
			case dword:
			case sse_ss: return "dword";
			case qword:
			case sse_sd: return "qword";
			case sse_ps:
			case sse_pd: return "xmmword";
			default:     return "??";
		}
	}
}
