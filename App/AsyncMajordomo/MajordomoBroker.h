#ifndef ASYNC_MAJORDOMO_BROKER_H_
#define ASYNC_MAJORDOMO_BROKER_H_

#include <atomic>
#include <chrono>
#include <deque>
#include <string>
#include <unordered_map>

#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <MajordomoClientData.h>
#include <MajordomoWorkerData.h>

struct PendingRequest {
	std::string clientAddr;
	MajordomoClientCmd::Request request;
};

struct Worker {
	std::string identity;
	std::chrono::steady_clock::time_point expiry;
	std::string serviceName;
};

struct Service {
	std::deque<PendingRequest> pendingRequests;
	std::deque<std::string> idleWorkers;
};

class MajordomoBroker {
  public:
	MajordomoBroker( const std::string &frontend, const std::string &backend );

	void bind();

	void run();

	void stop();

	void addWorker();

	void purgeExpiredWorker();

	void sendHeartbeat();

	void handleClientRequest();

	void handleWorkerResponse();

  private:
	void forward2Worker( const PendingRequest &request, const std::string &workerId );

	void forward2Client();

	void handleCmd( MajordomoWorkerCmd::MessageType msgType,
	                const std::string &workerIdentity,
	                const MajordomoWorkerCmd::Frames &frames );

	void handleHeartbeat( const std::string &workerIdentity, const MajordomoWorkerCmd::Frames &frames );

	void refreshHeartbeat( const std::string &workerIdentity );

	void handleReady( const std::string &workerIdentity, const MajordomoWorkerCmd::Frames &frames );

	void handleReply( const std::string &workerIdentity, const MajordomoWorkerCmd::Frames &frames );

	void handleDisconnect( const std::string &workerIdentity );

	void sendPendingRequests( Service &service );

	void disconnect( const std::string &workerIdentity );

	zmq::context_t mZmqCtx{ 1 };
	std::atomic_bool mIsRunning{ false };
	zmq::active_poller_t mPoller;
	std::unique_ptr<zmq::socket_t> mFrontSocket;
	std::unique_ptr<zmq::socket_t> mBackSocket;
	const std::string mFrontend;
	const std::string mBackend;
	std::unordered_map<std::string, Worker> mRegisteredWorkers; // workerId -> registered workers
	std::unordered_map<std::string, Service> mServices;         // serviceName -> idle workers
	std::chrono::steady_clock::time_point mHeartbeatDeadline;
};

#endif