#ifndef FREELANCE_SERVER_H_
#define FREELANCE_SERVER_H_

#include <zmq.hpp>
#include <zmq_addon.hpp>

class FreelanceServer {
  public:
	explicit FreelanceServer( const std::string &port );
	void run();
	void handleIncomingMessage();

  private:
	void setup( const std::string &endpoint );

	zmq::context_t mZmqCtx;
	zmq::socket_t mSocket;
};

#endif