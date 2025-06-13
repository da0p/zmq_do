#ifndef BROKER_STATE_H_
#define BROKER_STATE_H_
#include <queue>
#include <string>
#include <unordered_map>

#include <zmq.hpp>
#include <zmq_addon.hpp>

class BrokerState {
  public:
	explicit BrokerState( zmq::context_t &ctx );

	void setupSelf( std::string_view name );

	void setupPeers( const std::vector<std::string> &peerNames );

	void add2Poller( zmq::active_poller_t &poller );

	void broadcastMyState();

	void addIdleWorker( std::string_view name );

	[[nodiscard]] bool isLocalWorkerAvailable();

	[[nodiscard]] std::string getLocalWorker();

	[[nodiscard]] bool isRemotePeerAvailable();

	[[nodiscard]] std::string getRemotePeer();

  private:
	void handleStateUpdate();
	std::string mMyId;
	zmq::socket_t mStateBackend;
	zmq::socket_t mStateFrontend;
	std::queue<std::string> mMyIdleWorkers;
	std::unordered_map<std::string, uint32_t> mIdlePeers;
};

#endif