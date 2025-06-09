#ifndef STOP_TIMER_H_
#define STOP_TIMER_H_

#include <chrono>

class StopTimer {
  public:
	StopTimer() = default;
	void start() {
		mStart = std::chrono::steady_clock::now();
	}

	[[nodiscard]] std::chrono::milliseconds getTimeElasped() {
		auto duration = std::chrono::steady_clock::now() - mStart;
		return std::chrono::duration_cast<std::chrono::milliseconds>( duration );
	}

  private:
	std::chrono::steady_clock::time_point mStart;
};

#endif
