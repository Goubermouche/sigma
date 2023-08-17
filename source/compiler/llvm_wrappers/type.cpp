#include "type.h"
#include <utility/macros.h>

namespace sigma {
	type::type(base base, u16 pointer_level)
		: m_base(base), m_pointer_level(pointer_level) {}

	type::type(token tok, u16 pointer_level)
		: m_pointer_level(pointer_level) {
		static const std::unordered_map<token, base> token_to_type_map = {
			{ token::keyword_type_i8  , base::i8        },
			{ token::keyword_type_i16 , base::i16       },
			{ token::number_signed    , base::i32       },
			{ token::keyword_type_i32 , base::i32       },
			{ token::keyword_type_i64 , base::i64       },
			{ token::keyword_type_u8  , base::u8        },
			{ token::keyword_type_u16 , base::u16       },
			{ token::number_unsigned  , base::u32       },
			{ token::keyword_type_u32 , base::u32       },
			{ token::keyword_type_u64 , base::u64       },
			{ token::number_f32       , base::f32       },
			{ token::keyword_type_f32 , base::f32       },
			{ token::number_f64       , base::f64       },
			{ token::keyword_type_f64 , base::f64       },
			{ token::keyword_type_void, base::empty     },
			{ token::keyword_type_char, base::character },
			{ token::keyword_type_bool, base::boolean   },
		};

		const auto it = token_to_type_map.find(tok);
		ASSERT(it != token_to_type_map.end(), "");
		m_base = it->second;
	}

	type type::unknown() {
		return type(base::unknown, 0);
	}

	bool type::is_pointer() const {
		return m_pointer_level > 0;
	}

	bool type::is_unknown() const {
		return m_base == base::unknown;
	}

	type type::get_element_type() const {
		ASSERT(m_pointer_level > 0, "cannot get an element type of a non pointer type");
		return { m_base, static_cast<u16>(m_pointer_level - 1) };
	}

	type type::get_pointer_type() const	{
		return { m_base, static_cast<u16>(m_pointer_level + 1) };
	}

	void type::set_pointer_level(u16 pointer_level)	{
		m_pointer_level = pointer_level;
	}

	u16 type::get_pointer_level() const	{
		return m_pointer_level;
	}

	u16 type::get_bit_width() const	{
		if(m_pointer_level > 0) {
			return sizeof(i32*) * 8;
		}

		// todo: get type bit widths when compiling
		static const std::unordered_map<base, u16> type_to_bit_width = {
			{ base::i8       , 8  },
			{ base::i16      , 16 },
			{ base::i32      , 32 },
			{ base::i64      , 64 },
			{ base::u8       , 8  },
			{ base::u16      , 16 },
			{ base::u32      , 32 },
			{ base::u64      , 64 },
			{ base::f32      , 32 },
			{ base::f64      , 64 },
			{ base::character, 8  },
			{ base::boolean  , 8  }, // the actual bit width is 1, but we have to consider the default alignment of 8 bits 
		};

		const auto it = type_to_bit_width.find(m_base);
		ASSERT(it != type_to_bit_width.end(), "cannot get the bit width of the given type");
		return it->second;
	}

	type::base type::get_base() const {
		return m_base;
	}

	bool type::is_signed() const {
		switch (m_base) {
		case base::i8:
		case base::i16:
		case base::i32:
		case base::i64:
		case base::f32:
		case base::f64:
			return true;
		default:
			return false;
		}
	}

	bool type::is_unsigned() const {
		// consider pointers as unsigned
		if (m_pointer_level > 0) {
			return true;
		}

		switch (m_base) {
		case base::u8:
		case base::u16:
		case base::u32:
		case base::u64:
			return true;
		default:
			return false;
		}
	}

	bool type::is_floating_point() const {
		return m_base == base::f32 || m_base == base::f64;
	}

	bool type::is_integral() const {
		switch (m_base) {
		case base::i8:
		case base::i16:
		case base::i32:
		case base::i64:
		case base::u8:
		case base::u16:
		case base::u32:
		case base::u64:
		case base::boolean:
			return true;
		default:
			return false;
		}
	}

	bool type::is_numerical() const {
		// consider pointer types as a numerical type
		if(m_pointer_level > 0) {
			return true;
		}

		switch (m_base) {
		case base::i8:
		case base::i16:
		case base::i32:
		case base::i64:
		case base::u8:
		case base::u16:
		case base::u32:
		case base::u64:
		case base::f32:
		case base::f64:
			return true;
		default:
			return false;
		}
	}

	llvm::Type* type::get_llvm_type(llvm::LLVMContext& context) const {
		// pointer types
		if (m_pointer_level > 0) {
			switch (m_base) {
			case base::i8:
			case base::u8:
			case base::character:
				return llvm::Type::getInt8PtrTy(context);
			case base::i16:
			case base::u16:
				return llvm::Type::getInt16PtrTy(context);
			case base::i32:
			case base::u32:
				return llvm::Type::getInt32PtrTy(context);
			case base::i64:
			case base::u64:
				return llvm::Type::getInt64PtrTy(context);
			case base::f32:
				return llvm::Type::getFloatPtrTy(context);
			case base::f64:
				return llvm::Type::getDoublePtrTy(context);
			case base::empty:
				return llvm::Type::getInt8PtrTy(context);
			case base::boolean:
				return llvm::Type::getInt1PtrTy(context);
			default:
				ASSERT(false, "invalid base type");
				return nullptr;
			}
		}

		// element types 
		switch (m_base) {
		case base::i8:
		case base::u8:
		case base::character:
			return llvm::Type::getInt8Ty(context);
		case base::i16:
		case base::u16:
			return llvm::Type::getInt16Ty(context);
		case base::i32:
		case base::u32:
			return llvm::Type::getInt32Ty(context);
		case base::i64:
		case base::u64:
			return llvm::Type::getInt64Ty(context);
		case base::f32:
			return llvm::Type::getFloatTy(context);
		case base::f64:
			return llvm::Type::getDoubleTy(context);
		case base::empty:
			return llvm::Type::getVoidTy(context);
		case base::boolean:
			return llvm::Type::getInt1Ty(context);
		default:
			ASSERT(false, "invalid base type");
			return nullptr;
		}
	}

	std::string type::to_string() const {
		const static std::unordered_map<base, std::string> type_to_string_map = {
			{ base::i8           , "i8"            },
			{ base::i16          , "i16"           },
			{ base::i32          , "i32"           },
			{ base::i64          , "i64"           },
			{ base::u8           , "u8"            },
			{ base::u16          , "u16"           },
			{ base::u32          , "u32"           },
			{ base::u64          , "u64"           },
			{ base::f32          , "f32"           },
			{ base::f64          , "f64"           },
			{ base::function     , "function"      },
			{ base::function_call, "function_call" },
			{ base::empty        , "void"          },
			{ base::character    , "char"          },
			{ base::boolean      , "bool"          },
			{ base::unknown      , "unknown"       },
		};

		const auto it = type_to_string_map.find(m_base);
		ASSERT(it != type_to_string_map.end(), "cannot convert the given type to a string");
		return it->second + std::string(m_pointer_level, '*');
	}

	bool type::operator==(const type& other) const {
		return other.m_pointer_level == m_pointer_level && other.m_base == m_base;
	}

	bool is_token_type(token tok) {
		switch(tok) {
		case token::keyword_type_i8:
		case token::keyword_type_i16:
		case token::keyword_type_i32:
		case token::keyword_type_i64:
		case token::keyword_type_u8:
		case token::keyword_type_u16:
		case token::keyword_type_u32:
		case token::keyword_type_u64:
		case token::keyword_type_f32:
		case token::keyword_type_f64:
		case token::keyword_type_void:
		case token::keyword_type_char:
		case token::keyword_type_bool:
			return true;
		default:
			return false;
		}
	}
	type get_highest_precision_type(const type& left_type, const type& right_type) {
		assert(left_type != type::function && left_type != type::function_call &&
			right_type != type::function && right_type != type::function_call &&
			"invalid types for get_highest_precision_type");

		if (left_type.is_floating_point() || right_type.is_floating_point()) {
			return (left_type.get_base() == type::base::f64 || right_type.get_base() == type::base::f64) ? type(type::base::f64, 0) : type(type::base::f32, 0);
		}

		static std::unordered_map<type::base, i32> type_priority = {
			// { type::base::boolean, 1 }, { type::base::boolean, 1 },
			{ type::base::i8     , 1 }, { type::base::u8     , 1 },
			{ type::base::i16    , 2 }, { type::base::u16    , 2 },
			{ type::base::i32    , 3 }, { type::base::u32    , 3 },
			{ type::base::i64    , 4 }, { type::base::u64    , 4 }
		};

		return type_priority[left_type.get_base()] > type_priority[right_type.get_base()] ? left_type : right_type;
	}
}