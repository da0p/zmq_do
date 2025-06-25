#ifndef FREELANCE_HEARTBEAT_SERVICE_H_
#define FREELANCE_HEARTBEAT_SERVICE_H_

#include <chrono>
#include <string>

#include <zmq.hpp>
#include <zmq_addon.hpp>

class HeartbeatService {
  public:
	explicit HeartbeatService( zmq::context_t &ctx, const std::string &endpoint, std::chrono::milliseconds hbInterval );

	void run();

  private:
	void setup( const std::string &endpoint );
	void sendHeartbeat();

	zmq::socket_t mSocket;
	std::chrono::milliseconds mHeartbeatInterval;
	std::chrono::steady_clock::time_point mDeadline;
};

#endif