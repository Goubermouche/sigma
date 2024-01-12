#pragma once

#include <array>
#include <any>
#include <cstddef>
#include <codecvt>
#include <queue>
#include <deque>
#include <filesystem>
#include <fstream>
#include <future>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <set>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <regex>
#include <random>
#include <locale>
#include <numeric>
#include <map>
#include <variant>
#include <typeindex>
#include <type_traits>
#include <bitset>
#include <cstdlib>
#include <cstring>
#include <memory>

// platform specific 
#ifdef _WIN32
#define NOMINMAX
    #include <io.h>
	#include <fcntl.h>
#else
    #include <unistd.h>
	#include <signal.h>
#endif

namespace utility {
	namespace types {
		// signed integers
		using i8 = int8_t;
		using i16 = int16_t;
		using i32 = int32_t;
		using i64 = int64_t;

		// unsigned integers
		using u8 = uint8_t;
		using u16 = uint16_t;
		using u32 = uint32_t;
		using u64 = uint64_t;

		// floating point
		using f32 = float;
		using f64 = double;

    using ptr_diff = ptrdiff_t;

    template<typename type>
    using s_ptr = std::shared_ptr<type>;

    template<typename type>
    using u_ptr = std::unique_ptr<type>;
	} // namespace utility::types

	using namespace types;

	template <class type>
	auto num_digits(type number) -> u8 {
		u8 digits = 0;

		while (number) {
			number /= 10;
			digits++;
		}

		return digits;
	}

	namespace detail {
    template<typename type>
    struct has_const_iterator {
    private:
        typedef char yes;
        typedef struct { char array[2]; } no;

        template<typename container_type> static yes test(typename container_type::const_iterator*);
        template<typename container_type> static no  test(...);
    public:
        static const bool value = sizeof(test<type>(0)) == sizeof(yes);
    };

    template <typename type>
    struct has_begin_end {
      template<typename container_type> static char(&f(typename std::enable_if<
          std::is_same<decltype(static_cast<typename container_type::const_iterator(container_type::*)() const>(&container_type::begin)),
          typename container_type::const_iterator(container_type::*)() const>::value, void>::id*))[1];

      template<typename container_type> static char(&f(...))[2];

      template<typename container_type> static char(&g(typename std::enable_if<
          std::is_same<decltype(static_cast<typename container_type::const_iterator(container_type::*)() const>(&container_type::end)),
          typename container_type::const_iterator(container_type::*)() const>::value, void>::id*))[1];

      template<typename container_type> static char(&g(...))[2];

      static bool const beg_value = sizeof(f<type>(0)) == 1;
      static bool const end_value = sizeof(g<type>(0)) == 1;
    };

    template<typename type>
    struct is_container : std::integral_constant<bool, has_const_iterator<type>::value&& has_begin_end<type>::beg_value&& has_begin_end<type>::end_value> {};
	} // namespace detail
} // namespace utility
