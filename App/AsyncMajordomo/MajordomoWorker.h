#ifndef ASYNC_MAJORDOMO_WORKER_H_
#define ASYNC_MAJORDOMO_WORKER_H_

#include <atomic>
#include <chrono>
#include <string>

#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <MajordomoWorkerData.h>

class MajordomoWorker {
  public:
	explicit MajordomoWorker( const std::string &remoteAddr, const std::string &service, std::chrono::milliseconds selfHeartbeat );

	void stop();

	void connect();

	void receive();

  private:
	void handleIncomingMessage();
	void handleCmd( MajordomoWorkerCmd::MessageType msgType, const MajordomoWorkerCmd::Frames &frames );
	void handleRequest( const MajordomoWorkerCmd::Frames &frames );
	void updateHeartbeat();
	void checkRemoteHeartbeat();

	zmq::context_t mZmqCtx{ 1 };
	uint32_t mMissingHeartbeat{ 0 };
	std::atomic_bool mIsRunning{ false };
	const std::string mRemoteAddr;
	const std::string mService;
	std::unique_ptr<zmq::socket_t> mSocket;
	zmq::active_poller_t mPoller;
	std::chrono::milliseconds mSelfHeartbeatPeriod;
	std::chrono::steady_clock::time_point mHeartbeatDeadline;
};

#endif
