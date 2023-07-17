#include "timer.h"

namespace sigma {
	void timer::start()	{
		m_start = std::chrono::steady_clock::now();
	}

	f64 timer::elapsed() const {
		const auto now = std::chrono::steady_clock::now();
		const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_start);
		return static_cast<double>(duration.count());
	}
}