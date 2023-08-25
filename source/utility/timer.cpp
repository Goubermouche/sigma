#include "timer.h"

namespace utility {
	void timer::start()	{
		m_start = std::chrono::high_resolution_clock::now();
	}
}