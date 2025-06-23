#include "MajordomoBroker.h"

#include <spdlog/spdlog.h>
#include <zmq.hpp>

#include <ZmqUtil.h>

#include <MajordomoWorkerMessage.h>

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

	// ZmqUtil::dump( rawFrames.value() );
	std::string clientAddr{ rawFrames.value()[ 0 ].begin(), rawFrames.value()[ 0 ].end() };
	MajordomoClientMessage::Frames frames{ rawFrames.value().begin() + 1, rawFrames.value().end() };
	auto discoveryRequest = MajordomoClientMessage::DiscoveryRequest::from( frames );
	if ( discoveryRequest.has_value() ) {
		spdlog::info( "Received discovery request for service={}", discoveryRequest.value().serviceName );
		handleDiscovery( discoveryRequest.value(), clientAddr );
		return;
	}

	auto commandRequest = MajordomoClientMessage::Request::from( frames );
	if ( !commandRequest.has_value() ) {
		spdlog::error( "Failed to parse frames!" );
		ZmqUtil::dump( rawFrames.value() );
		return;
	}

	if ( !mServices.contains( commandRequest.value().serviceName ) ) {
		spdlog::warn( "Service {} does not exist or not registered!", commandRequest.value().serviceName );
		return;
	}

	auto &service = mServices.at( commandRequest.value().serviceName );
	service.pendingRequests.emplace_back( clientAddr, commandRequest.value() );
	sendPendingRequests( service );
}

void MajordomoBroker::handleDiscovery( const MajordomoClientMessage::DiscoveryRequest &request, const std::string &clientAddr ) {
	if ( mServices.contains( request.serviceName ) ) {
		// 200 - ok
		MajordomoClientMessage::DiscoveryReply reply{ .header = gDiscoveryService,
			                                          .serviceName = request.serviceName,
			                                          .status = "Ok" };
		MajordomoClientMessage::Frames frames = MajordomoClientMessage::DiscoveryReply::to( reply, clientAddr );
		ZmqUtil::sendAllFrames( *mFrontSocket, frames );
	} else {
		// 404 - not found
		MajordomoClientMessage::DiscoveryReply reply{ .header = gDiscoveryService,
			                                          .serviceName = request.serviceName,
			                                          .status = "NotFound" };
		MajordomoClientMessage::Frames frames = MajordomoClientMessage::DiscoveryReply::to( reply, clientAddr );
		ZmqUtil::sendAllFrames( *mFrontSocket, frames );
	}
}

void MajordomoBroker::sendPendingRequests( Service &service ) {
	while ( !service.idleWorkers.empty() && !service.pendingRequests.empty() ) {
		auto idleWorkerId = service.idleWorkers.front();
		service.idleWorkers.pop_front();
		auto pendingReq = service.pendingRequests.front();
		service.pendingRequests.pop_front();
		forward2Worker( pendingReq, idleWorkerId );
	}
}

void MajordomoBroker::forward2Worker( const PendingRequest &request, const std::string &workerId ) {
	MajordomoWorkerMessage::Request forwardedReq{ request.request.version, request.clientAddr, request.request.body };
	auto requestFrames = MajordomoWorkerMessage::Request::to( forwardedReq, workerId );
	spdlog::debug( "forward request from clientAddr: {} to worker: {}", request.clientAddr, workerId );
	ZmqUtil::sendAllFrames( *mBackSocket, requestFrames );
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

	handleCmd( static_cast<MajordomoWorkerMessage::MessageType>( frames[ 2 ].front() ), workerIdentity, frames );
}

void MajordomoBroker::handleCmd( MajordomoWorkerMessage::MessageType msgType,
                                 const std::string &workerIdentity,
                                 const MajordomoWorkerMessage::Frames &frames ) {
	switch ( msgType ) {
		case MajordomoWorkerMessage::MessageType::Heartbeat:
			handleHeartbeat( workerIdentity, frames );
			break;
		case MajordomoWorkerMessage::MessageType::Ready:
			handleReady( workerIdentity, frames );
			break;
		case MajordomoWorkerMessage::MessageType::Reply:
			handleReply( workerIdentity, frames );
			break;
		case MajordomoWorkerMessage::MessageType::Disconnect:
			handleDisconnect( workerIdentity );
			break;
		case MajordomoWorkerMessage::MessageType::Request:
		default:
			spdlog::error( "Invalid command response received!" );
			break;
	}
}

void MajordomoBroker::handleDisconnect( const std::string &workerIdentity ) {
	if ( !mRegisteredWorkers.contains( workerIdentity ) ) {
		spdlog::debug( "Worker {} already removed", workerIdentity );
		return;
	}

	auto &worker = mRegisteredWorkers[ workerIdentity ];
	if ( !mServices.contains( worker.serviceName ) ) {
		spdlog::debug( "Service name: {} is not registered", worker.serviceName );
		return;
	}

	auto &service = mServices[ worker.serviceName ];
	std::erase_if( service.idleWorkers, [ &workerIdentity ]( auto &&workerId ) { return workerIdentity == workerId; } );
}

void MajordomoBroker::handleReply( const std::string &workerIdentity, const MajordomoWorkerMessage::Frames &frames ) {
	auto rawWorkerReply = MajordomoWorkerMessage::Reply::from( frames );
	if ( !rawWorkerReply.has_value() ) {
		spdlog::error( "Failed to parse reply message" );
		return;
	}

	if ( !mRegisteredWorkers.contains( workerIdentity ) ) {
		spdlog::warn( "Received a reply from unregistered worker: {}", workerIdentity );
		disconnect( workerIdentity );
		return;
	}
	auto &serviceName = mRegisteredWorkers.at( workerIdentity ).serviceName;
	if ( !mServices.contains( serviceName ) ) {
		spdlog::error( "No service: `{}`", serviceName );
		return;
	}

	// return this worker to the pool
	auto &service = mServices.at( serviceName );
	service.idleWorkers.push_back( workerIdentity );

	auto workerReply = rawWorkerReply.value();
	MajordomoClientMessage::Reply clientReply{ workerReply.version, serviceName, workerReply.body };
	MajordomoClientMessage::Frames replyFrames = MajordomoClientMessage::Reply::to( clientReply, workerReply.clientAddr );
	ZmqUtil::sendAllFrames( *mFrontSocket, replyFrames );
	// send new request if there is any pending
	sendPendingRequests( service );
}

void MajordomoBroker::handleHeartbeat( const std::string &workerIdentity, const MajordomoWorkerMessage::Frames &frames ) {
	auto rawHeartbeat = MajordomoWorkerMessage::Heartbeat::from( frames );
	if ( !rawHeartbeat.has_value() ) {
		spdlog::error( "Failed to parse heartbeat message!" );
		return;
	}

	if ( !mRegisteredWorkers.contains( workerIdentity ) ) {
		spdlog::warn( "Received heartbeat from an unknown worker: {}", workerIdentity );
		disconnect( workerIdentity );
		return;
	}
	refreshHeartbeat( workerIdentity );
}

void MajordomoBroker::disconnect( const std::string &workerIdentity ) {
	MajordomoWorkerMessage::Disconnect disconnect{ .version = gMajVer };
	MajordomoWorkerMessage::Frames disconnectFrames = MajordomoWorkerMessage::Disconnect::to( disconnect, workerIdentity );
	ZmqUtil::sendAllFrames( *mBackSocket, disconnectFrames );
}

void MajordomoBroker::refreshHeartbeat( const std::string &workerIdentity ) {
	if ( mRegisteredWorkers.contains( workerIdentity ) ) {
		spdlog::debug( "Refresh heartbeat for worker: {}", workerIdentity );
		mRegisteredWorkers[ workerIdentity ].expiry = std::chrono::steady_clock::now() + gWorkerHeartbeatExpiryDuration;
	} else {
		spdlog::warn( "Received heartbeat from unknown worker: workerId = {}", workerIdentity );
	}
}

void MajordomoBroker::handleReady( const std::string &workerId, const MajordomoWorkerMessage::Frames &frames ) {
	auto rawReady = MajordomoWorkerMessage::Ready::from( frames );
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

	// send request if there is any pending
	auto &service = mServices[ ready.serviceName ];
	sendPendingRequests( service );
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
			MajordomoWorkerMessage::Heartbeat heartbeat{ .version = gMajVer };
			auto heartbeatFrames = MajordomoWorkerMessage::Heartbeat::to( heartbeat, workerId );
			ZmqUtil::sendAllFrames( *mBackSocket, heartbeatFrames );
		}
		mHeartbeatDeadline = now + gBrokerHeartbeatInterval;
	}
}

void MajordomoBroker::stop() {
	mIsRunning = false;
}