#include "utility/types.h"

namespace sigma::detail {
    std::string format_ending(
        u64 count,
        const std::string& singular,
        const std::string& plural
    ) {
        return count == 1 ? singular : plural;
    }
}