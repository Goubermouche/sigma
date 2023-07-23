#pragma once
#include "utility/macros.h"

namespace sigma {
	class timer {
	public:
		void start();

		template<typename duration>
		f64 elapsed() const {
			const auto now = std::chrono::high_resolution_clock::now();
			const auto diff = now - m_start;
			return std::chrono::duration_cast<duration>(diff).count();
		}
	private:
		std::chrono::high_resolution_clock::time_point m_start;
	};
}
