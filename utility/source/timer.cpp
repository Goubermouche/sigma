#include "timer.h"

namespace sigma {
	void timer::start()	{
		m_start = std::chrono::high_resolution_clock::now();
	}
}