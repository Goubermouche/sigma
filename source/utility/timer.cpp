#include "timer.h"

namespace utility {
	void timer::start()	{
		m_start = std::chrono::high_resolution_clock::now();
	}

  f64 timer::elapsed_seconds() const {
		return elapsed<std::chrono::duration<f64>>();
  }
}