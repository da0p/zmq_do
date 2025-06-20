#include "MajordomoBroker.h"

#include <spdlog/spdlog.h>
#include <zmq.hpp>

#include <ZmqUtil.h>

#include <MajordomoWorkerData.h>

MajordomoBroker::MajordomoBroker( const std::string &frontend, const std::string &backend ) :
        mFrontend{ frontend }, mBackend{ backend } {
	bind();
}

void MajordomoBroker::bind() {
	mFrontSocket = std::make_unique<zmq::socket_t>( mZmqCtx, zmq::socket_type::router );
	mBackSocket = std::make_unique<zmq::socket_t>( mZmqCtx, zmq::socket_type::router );

	mPoller.add( *mBackSocket, zmq::event_flags::pollin, [ this ]( zmq::event_flags flag ) { handleWorkerResponse(); } );
	mPoller.add( *mFrontSocket, zmq::event_flags::pollin, [ this ]( zmq::event_flags flag ) { handleClientRequest(); } );

	spdlog::info( "Binding frontend to address: {}, and backend to address: {}", mFrontend, mBackend );
	mFrontSocket->bind( mFrontend );
	mBackSocket->bind( mBackend );
}

void MajordomoBroker::handleClientRequest() {
	auto rawFrames = ZmqUtil::recvAllFrames( *mFrontSocket );
	if ( !rawFrames.has_value() ) {
		spdlog::error( "Failed to receive frames from frontend!" );
		return;
	}

	std::string clientAddr{ rawFrames.value()[ 0 ].begin(), rawFrames.value()[ 0 ].end() };
	MajordomoClientCmd::Frames frames{ rawFrames.value().begin() + 2, rawFrames.value().end() };
	auto rawRequest = MajordomoClientCmd::Request::from( frames );
	if ( !rawRequest.has_value() ) {
		spdlog::error( "Failed to parse frames!" );
		return;
	}

	auto request = rawRequest.value();
	if ( !mServices.contains( request.serviceName ) ) {
		spdlog::warn( "Service {} does not exist or not registered!", request.serviceName );
		return;
	}

	auto &service = mServices.at( request.serviceName );
	service.pendingRequests.emplace_back( clientAddr, request );
	if ( !service.idleWorkers.empty() ) {
		while ( !service.idleWorkers.empty() && !service.pendingRequests.empty() ) {
			auto idleWorkerId = service.idleWorkers.front();
			service.idleWorkers.pop_front();
			auto pendingReq = service.pendingRequests.front();
			service.pendingRequests.pop_front();
			forward2Worker( pendingReq, idleWorkerId );
		}
	}
}

void MajordomoBroker::forward2Worker( const PendingRequest &request, const std::string &workerId ) {
	MajordomoWorkerCmd::Request forwardedReq{ request.request.version, request.clientAddr, request.request.body };
	auto requestCommand = MajordomoWorkerCmd::Request::to( forwardedReq );

	MajordomoWorkerCmd::Frame workerIdentity{ workerId.begin(), workerId.end() };
	MajordomoWorkerCmd::Frames frames{ workerIdentity };

	std::copy( requestCommand.begin(), requestCommand.end(), std::back_inserter( frames ) );
	spdlog::debug( "forward request to worker: {}", workerId );
	ZmqUtil::sendAllFrames( *mBackSocket, frames );
}

void MajordomoBroker::handleWorkerResponse() {
	auto rawFrames = ZmqUtil::recvAllFrames( *mBackSocket );
	if ( !rawFrames.has_value() ) {
		spdlog::error( "Failed to receive frames from backend!" );
		return;
	}

	auto frames = rawFrames.value();
	std::string workerIdentity{ frames[ 0 ].begin(), frames[ 0 ].end() };
	frames.erase( frames.begin() );

	if ( frames.size() < 3 ) {
		spdlog::error( "Invalid message received!" );
		ZmqUtil::dump( frames );
		return;
	}

	if ( frames[ 2 ].size() != 1 ) {
		spdlog::error( "Invalid message type received!" );
		ZmqUtil::dump( frames );
		return;
	}

	handleCmd( static_cast<MajordomoWorkerCmd::MessageType>( frames[ 2 ].front() ), workerIdentity, frames );
}

void MajordomoBroker::handleCmd( MajordomoWorkerCmd::MessageType msgType,
                                 const std::string &workerIdentity,
                                 const MajordomoWorkerCmd::Frames &frames ) {
	switch ( msgType ) {
		case MajordomoWorkerCmd::MessageType::Heartbeat:
			handleHeartbeat( workerIdentity, frames );
			break;
		case MajordomoWorkerCmd::MessageType::Ready:
			handleReady( workerIdentity, frames );
			break;
		case MajordomoWorkerCmd::MessageType::Reply:
			handleReply( workerIdentity, frames );
			break;
		case MajordomoWorkerCmd::MessageType::Request:
		case MajordomoWorkerCmd::MessageType::Disconnect:
		default:
			spdlog::error( "Invalid command response received!" );
			break;
	}
}

void MajordomoBroker::handleReply( const std::string &workerIdentity, const MajordomoWorkerCmd::Frames &frames ) {
	auto rawWorkerReply = MajordomoWorkerCmd::Reply::from( frames );
	if ( !rawWorkerReply.has_value() ) {
		spdlog::error( "Failed to parse reply message" );
		return;
	}

	if ( !mRegisteredWorkers.contains( workerIdentity ) ) {
		spdlog::warn( "Received a reply from unregistered worker: {}", workerIdentity );
		return;
	}
	auto &serviceName = mRegisteredWorkers.at( workerIdentity ).serviceName;
	if ( !mServices.contains( serviceName ) ) {
		spdlog::error( "No service: `{}`", serviceName );
		return;
	}

	// return this worker to the pool
	mServices.at( serviceName ).idleWorkers.push_back( workerIdentity );

	auto workerReply = rawWorkerReply.value();
	MajordomoClientCmd::Reply clientReply{ workerReply.version, serviceName, workerReply.body };
	MajordomoClientCmd::Frames clientReplyFrames = MajordomoClientCmd::Reply::to( clientReply );

	MajordomoClientCmd::Frame routingId{ workerReply.clientAddr.begin(), workerReply.clientAddr.end() };
	MajordomoClientCmd::Frames replyFrames{ routingId };
	std::copy( clientReplyFrames.begin(), clientReplyFrames.end(), std::back_inserter( replyFrames ) );
	ZmqUtil::sendAllFrames( *mFrontSocket, replyFrames );
}

void MajordomoBroker::handleHeartbeat( const std::string &workerIdentity, const MajordomoWorkerCmd::Frames &frames ) {
	auto rawHeartbeat = MajordomoWorkerCmd::Heartbeat::from( frames );
	if ( !rawHeartbeat.has_value() ) {
		spdlog::error( "Failed to parse heartbeat message!" );
		return;
	}
	refreshHeartbeat( workerIdentity );
}

void MajordomoBroker::refreshHeartbeat( const std::string &workerIdentity ) {
	if ( mRegisteredWorkers.contains( workerIdentity ) ) {
		spdlog::debug( "Refresh heartbeat for worker: {}", workerIdentity );
		mRegisteredWorkers[ workerIdentity ].expiry = std::chrono::steady_clock::now() + gWorkerHeartbeatExpiryDuration;
	} else {
		spdlog::warn( "Received heartbeat from unknown worker: workerId = {}", workerIdentity );
	}
}

void MajordomoBroker::handleReady( const std::string &workerId, const MajordomoWorkerCmd::Frames &frames ) {
	auto rawReady = MajordomoWorkerCmd::Ready::from( frames );
	if ( !rawReady.has_value() ) {
		spdlog::error( "Failed to parse ready message!" );
		return;
	}
	auto ready = rawReady.value();

	Worker worker{ .identity = workerId,
		           .expiry = std::chrono::steady_clock::now() + gWorkerHeartbeatExpiryDuration,
		           .serviceName = ready.serviceName };
	spdlog::info( "Register worker `{}` for service `{}`", workerId, ready.serviceName );
	mRegisteredWorkers[ workerId ] = worker;
	mServices[ ready.serviceName ].idleWorkers.push_back( workerId );
}

void MajordomoBroker::run() {
	mIsRunning = true;
	mHeartbeatDeadline = std::chrono::steady_clock::now();
	while ( mIsRunning ) {
		[[maybe_unused]] auto evt = mPoller.wait( std::chrono::milliseconds( 100 ) );
		purgeExpiredWorker();
		sendHeartbeat();
	}
}

void MajordomoBroker::purgeExpiredWorker() {
	auto now = std::chrono::steady_clock::now();
	std::vector<Worker> expiredWorker;
	for ( const auto &[ workerId, worker ] : mRegisteredWorkers ) {
		if ( now > worker.expiry ) {
			expiredWorker.push_back( worker );
		}
	}

	for ( const auto &worker : expiredWorker ) {
		spdlog::debug( "Purge expired worker: {}", worker.identity );
		mRegisteredWorkers.erase( worker.identity );
		std::erase_if( mServices[ worker.serviceName ].idleWorkers,
		               [ &worker ]( auto &&workerId ) { return workerId == worker.identity; } );
	}
}

void MajordomoBroker::sendHeartbeat() {
	auto now = std::chrono::steady_clock::now();
	if ( now > mHeartbeatDeadline ) {
		for ( const auto &[ workerId, worker ] : mRegisteredWorkers ) {
			MajordomoWorkerCmd::Heartbeat heartbeat{ .version = gMajVer };
			auto heartbeatCmd = MajordomoWorkerCmd::Heartbeat::to( heartbeat );
			MajordomoWorkerCmd::Frame routingId{ workerId.begin(), workerId.end() };
			MajordomoWorkerCmd::Frames frames{ routingId };
			std::copy( heartbeatCmd.begin(), heartbeatCmd.end(), std::back_inserter( frames ) );
			ZmqUtil::sendAllFrames( *mBackSocket, frames );
		}
		mHeartbeatDeadline = now + gBrokerHeartbeatInterval;
	}
}

void MajordomoBroker::stop() {
	mIsRunning = false;
}