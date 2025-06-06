#include <chrono>

#include <ratio>
#include <zmq.hpp>
#include <spdlog/spdlog.h>

class Timer {
  public:
	Timer() = default;
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

int main( int argc, char *argv[] ) {
	zmq::context_t zmqContext;
	zmq::socket_t zmqSink{ zmqContext, zmq::socket_type::pull };
    zmqSink.bind("tcp://*:5558");

	// wait for start event
    Timer timer;
	zmq::message_t startEvent;
	[[maybe_unused]] auto _ = zmqSink.recv( startEvent, zmq::recv_flags::none );
    timer.start();

    // wait for 100 results
    for (auto i = 0; i < 100; i++) {
        zmq::message_t message;
        auto result = zmqSink.recv(message, zmq::recv_flags::none);
        if (result.has_value()) {
            spdlog::info("Receiving result {}", i);
        }
    }

    spdlog::info("Total elasped time: {} ms", timer.getTimeElasped().count());

	return 0;
}