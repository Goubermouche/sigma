#pragma once
#include "macros.h"

namespace utility {
	class timer {
	public:
		void start();

		template<typename duration>
		[[nodiscard]] f64 elapsed() const;

		[[nodiscard]] f64 elapsed_seconds() const;
	private:
		std::chrono::high_resolution_clock::time_point m_start;
	};

	template<typename duration>
	f64 timer::elapsed() const {
		const auto now = std::chrono::high_resolution_clock::now();
		const auto diff = now - m_start;
		return std::chrono::duration_cast<duration>(diff).count();
	}
}
