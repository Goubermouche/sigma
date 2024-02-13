#pragma once
#include "macros.h"

namespace utility {
	class timer {
	public:
		void start();

		template<typename duration>
		[[nodiscard]] auto elapsed() const -> f64 {
			const auto now = std::chrono::high_resolution_clock::now();
			const auto diff = now - m_start;
			return std::chrono::duration_cast<duration>(diff).count();
		}

		[[nodiscard]] f64 elapsed_seconds() const {
			return elapsed<std::chrono::duration<f64>>();
		}
	private:
		std::chrono::high_resolution_clock::time_point m_start;
	};
} // namespace utility
