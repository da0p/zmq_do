#include <chrono>

#include <zmq.hpp>
#include <zmq_addon.hpp>

#include "BinaryStarFsm.h"

struct Response {
	std::string clientId;
	std::string body;
};

class BinaryStarServer {
  public:
	explicit BinaryStarServer( bool primary,
	                           const std::string &handleReqSocket,
	                           const std::string &selfStatusPubSocket,
	                           const std::string &peerStatusSubSocket );

	void run();

  private:
	void setup( const std::string &handleReqSocket, const std::string &selfStatusPubSocket, const std::string &peerStatusSubSocket );
	void pubSelfState();
	void handlePeerStateReceived();
	void handleRequestReceived();
	[[nodiscard]] EventName toPeerEvt( StateName state );
	void respond();

	zmq::context_t mZmqCtx;
	zmq::active_poller_t mPoller;
	std::unique_ptr<zmq::socket_t> mHandleReqSocket;
	std::unique_ptr<zmq::socket_t> mSelfStatusPubSocket;
	std::unique_ptr<zmq::socket_t> mPeerStatusSubSocket;
	BinaryStarFsm mBinaryStarFsm;
	std::chrono::steady_clock::time_point mStatePubDeadline;
	Response mResponse;
};