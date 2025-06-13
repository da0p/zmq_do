#ifndef _INTER_BROKER_CLIENT_H_
#define _INTER_BROKER_CLIENT_H_
#include <atomic>
#include <thread>

#include <zmq.hpp>

class Client {
  public:
	explicit Client( zmq::context_t &ctx, const std::string &identity, uint32_t num );

	void run();

	void stop();

  private:
	void doRequest();

	std::string mIdentity;
	uint32_t mNum;
	zmq::socket_t mSocket;
	std::atomic_bool mIsRunning{ false };
	std::thread mRunningThread;
};

#endif