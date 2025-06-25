#ifndef FREELANCE_REQUEST_SERVICE_H_
#define FREELANCE_REQUEST_SERVICE_H_

#include <chrono>
#include <string>

#include <zmq.hpp>
#include <zmq_addon.hpp>

class RequestService {
  public:
	explicit RequestService( zmq::context_t &ctx, const std::string &endpoint, std::chrono::milliseconds interval );

	void run();

  private:
	void setup( const std::string &endpoint );

	zmq::socket_t mSocket;
	std::chrono::milliseconds mInterval;
};

#endif