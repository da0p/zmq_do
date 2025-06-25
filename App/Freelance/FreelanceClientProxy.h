#ifndef FREELANCE_CLIENT_PROXY_H_
#define FREELANCE_CLIENT_PROXY_H_

#include <unordered_map>

#include <zmq.hpp>
#include <zmq_addon.hpp>

class FreelanceProxy {
  public:
	explicit FreelanceProxy( zmq::context_t &ctx,
	                         const std::vector<uint16_t> &port,
	                         const std::string &hbEndpoint,
	                         const std::string &reqEndpoint );

	void run();

  private:
	void setupServers( const std::vector<uint16_t> &ports );
	void setupHeartbeatSocket( const std::string &hbEndpoint );
	void setupFrontSocket();
	void setupReqSocket( const std::string &reqEndpoint );

	void handleIncomingMessage();

	void forwardReq();
	void forwardHeartbeat();
	void refreshHeartbeat( const std::string &serverId );
	void purgeDeadServers();

	zmq::active_poller_t mPoller;
	zmq::socket_t mFrontSocket;
	zmq::socket_t mHeartbeatSocket;
	zmq::socket_t mReqSocket;
	std::unordered_map<std::string, std::chrono::steady_clock::time_point> mAvailServers;
	std::vector<std::string> mRegisteredServers;
};

#endif