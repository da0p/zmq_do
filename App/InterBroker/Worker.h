#ifndef INTER_BROKER_WORKER_H_
#define INTER_BROKER_WORKER_H_

#include <thread>

#include <zmq.hpp>

class Worker {
  public:
	explicit Worker( zmq::context_t &ctx, const std::string &identity, uint32_t num );

	void run();

	void stop();

  private:
	void doWork();

	std::string mIdentity;
	uint32_t mNum;
	zmq::socket_t mSocket;
	std::thread mRunningThread;
	std::atomic_bool mIsRunning{ false };
};

#endif