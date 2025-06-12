#ifndef BROKER_STATE_H_
#define BROKER_STATE_H_
#include <queue>
#include <string>
#include <unordered_map>

#include <zmq.hpp>

class BrokerState {
  public:
	explicit BrokerState( zmq::context_t &ctx );

	void setupSelf( std::string_view name );

	void setupPeers( const std::vector<std::string> &peerNames );

	void run();

  private:
	void broadcastMyState();

	std::string mMyId;
	zmq::socket_t mStateBackend;
	zmq::socket_t mStateFrontend;
	std::queue<std::string> mMyIdleWorkers;
	std::unordered_map<std::string, uint32_t> mOtherIdleWorkers;
};

#endif