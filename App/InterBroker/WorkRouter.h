#ifndef INTER_BROKER_WORK_ROUTER_H_
#define INTER_BROKER_WORK_ROUTER_H_
#include <atomic>
#include <queue>
#include <string>

#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <BrokerState.h>

class WorkRouter {
  public:
	explicit WorkRouter( zmq::context_t &ctx );

	void setupSelf( const std::string &myId );
	void setupRemote( const std::vector<std::string> &peers );

	void run();

  private:
	void handleLocalBackend();
	void handleRemoteBackend();
	void handleLocalFrontend();
	void handleRemoteFrontend();

	void handleCapacity( zmq::active_poller_t &poller );

	zmq::socket_t mLocalFrontendSocket;
	zmq::socket_t mLocalBackendSocket;
	zmq::socket_t mRemoteFrontendSocket;
	zmq::socket_t mRemoteBackendSocket;
	BrokerState mBrokerState;
	std::string mMyId;
	bool mIsPollingFront{ false };
	std::atomic_bool mIsRunning{ false };
};
#endif