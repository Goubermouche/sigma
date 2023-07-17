#pragma once
#include "utility/macros.h"

namespace sigma {
	class timer {
	public:
		void start();
		f64 elapsed() const;
	private:
		std::chrono::steady_clock::time_point m_start;
	};
}
