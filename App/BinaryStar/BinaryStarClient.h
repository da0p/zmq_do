#ifndef BINARY_STAR_CLIENT_H_
#define BINARY_STAR_CLIENT_H_

#include <zmq.hpp>
#include <zmq_addon.hpp>

class BinaryStarClient {
  public:
	explicit BinaryStarClient( const std::string &primaryAddr, const std::string &failoverAddr );
	void run();

  private:
	void connect();
	void setupCallback();

	zmq::context_t mZmqCtx;
	zmq::active_poller_t mPoller;
	std::unique_ptr<zmq::socket_t> mSocket;
	uint32_t mSeq{ 0 };
	bool mExpectReply{ false };
	std::array<std::string, 2> mAddresses;
	uint32_t mAddrTurn{ 0 };
};

#endif