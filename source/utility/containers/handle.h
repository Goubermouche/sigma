#pragma once
#include "utility/types.h"

namespace utility::types {
    // note: due to the lightweight nature of ptr, atomic operations are NOT used
    //       and thus the container is not thread-safe. 

    /**
     * \brief Simple and lightweight non-owning pointer abstraction.
     * \tparam type Type of the contained pointer
     */
    template<typename type>
	class handle {
	public:
        handle() = default;
        handle(type* ptr)
            : m_ptr(ptr) {}

        type& operator*() const {
            return *m_ptr;
        }

        type* operator->() const {
            return m_ptr;
        }

        type* get() const {
            return m_ptr;
        }

        bool operator==(const handle& other) const {
            return m_ptr == other.m_ptr;
        }

        operator bool() const noexcept {
            return m_ptr != nullptr;
        }
	private:
        type* m_ptr = nullptr;
	};
}

namespace std {
    template<typename type>
    struct hash<utility::types::handle<type>> {
        utility::u64 operator()(const utility::types::handle<type>& h) const noexcept {
            // hash the internal pointer
            return hash<type*>{}(h.get());
        }
    };

    template<typename type>
    struct formatter<utility::types::handle<type>> {
        auto parse(format_parse_context& ctx) {
            return ctx.begin();
        }

        auto format(const utility::types::handle<type>& obj, format_context& ctx) {
            if (obj) {
                return format_to(ctx.out(), "{}", (utility::u64)obj.get());
            }
            else {
                return format_to(ctx.out(), "0");
            }
        }
    };
}