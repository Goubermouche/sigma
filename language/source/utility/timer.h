#pragma once
#include "macros.h"

namespace channel {
	class timer {
	public:
		void start();
		double elapsed() const;
	private:
		std::chrono::steady_clock::time_point m_start;
	};
}