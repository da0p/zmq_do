#ifndef MAJORDOMO_CLIENT_H_
#define MAJORDOMO_CLIENT_H_

#include <atomic>
#include <chrono>
#include <optional>
#include <vector>

#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <MajordomoClientData.h>

class MajordomoClient {
  public:
	explicit MajordomoClient( const std::string &brokerAddr );

	void stop();
	void connect();
	std::optional<MajordomoClientCmd::Reply> send( const std::string &service,
	                                               const std::vector<uint8_t> &messageBody,
	                                               std::chrono::milliseconds timeout,
	                                               uint32_t retries );

  private:
	void handleResponse();

	zmq::context_t mZmqCtx{ 1 };
	zmq::active_poller_t mPoller;
	std::string mBrokerAddr;
	std::unique_ptr<zmq::socket_t> mSocket;
	std::atomic_bool mIsRunning{ false };
	std::optional<MajordomoClientCmd::Reply> mReply;
};

#endif