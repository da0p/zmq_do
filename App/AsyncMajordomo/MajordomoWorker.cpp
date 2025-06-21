#include "MajordomoWorkerData.h"
#include "ZmqUtil.h"
#include <MajordomoWorker.h>

#include <chrono>
#include <spdlog/spdlog.h>
#include <zmq.hpp>

#include <RandomString.h>

namespace {
	constexpr uint32_t gMaxMissingHeartbeats{ 3 };
}

MajordomoWorker::MajordomoWorker( const std::string &remoteAddr, const std::string &service, std::chrono::milliseconds selfHeartbeat ) :
        mRemoteAddr{ remoteAddr }, mService{ service }, mSelfHeartbeatPeriod{ selfHeartbeat } {
	connect();
}

void MajordomoWorker::stop() {
	spdlog::info( "Interrupt received. Kill worker!" );
	mIsRunning = false;
}

void MajordomoWorker::connect() {
	if ( mSocket ) {
		mPoller.remove( *mSocket );
		mSocket.reset();
	}
	mSocket = std::make_unique<zmq::socket_t>( mZmqCtx, zmq::socket_type::dealer );
	auto identity = RandomString::generate( 5 );
	mSocket->set( zmq::sockopt::routing_id, identity );
	mSocket->set( zmq::sockopt::linger, 0 );
	mPoller.add( *mSocket, zmq::event_flags::pollin, [ this ]( zmq::event_flags flag ) { handleIncomingMessage(); } );

	spdlog::info( "Connecting to broker at {} with identity {}", mRemoteAddr, identity );
	mSocket->connect( mRemoteAddr );

	spdlog::info( "Sending `ready` message" );
	MajordomoWorkerCmd::Ready ready{ .version = gMajVer, .serviceName = mService };
	ZmqUtil::sendAllFrames( *mSocket, MajordomoWorkerCmd::Ready::to( ready ) );
	mHeartbeatDeadline = std::chrono::steady_clock::now() + mSelfHeartbeatPeriod;
	mMissingHeartbeat = 0;
}

void MajordomoWorker::handleIncomingMessage() {
	auto rawFrames = ZmqUtil::recvAllFrames( *mSocket );
	if ( !rawFrames.has_value() ) {
		spdlog::error( "Failed to receive request message!" );
		return;
	}
	auto frames = rawFrames.value();
	mMissingHeartbeat = 0;
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

	handleCmd( static_cast<MajordomoWorkerCmd::MessageType>( frames[ 2 ].front() ), frames );
}

void MajordomoWorker::handleCmd( MajordomoWorkerCmd::MessageType msgType, const MajordomoWorkerCmd::Frames &frames ) {
	switch ( msgType ) {
		case MajordomoWorkerCmd::MessageType::Request:
			handleRequest( frames );
			break;
		case MajordomoWorkerCmd::MessageType::Disconnect:
			connect();
			break;
		case MajordomoWorkerCmd::MessageType::Heartbeat:
			// we already reset missing heartbeat to 0 whenever a message is
			// received no need to handle anything here
			spdlog::info( "Received heartbeat from broker" );
			break;
		default:
			spdlog::error( "Invalid message type received" );
			break;
	}
}

void MajordomoWorker::handleRequest( const MajordomoWorkerCmd::Frames &frames ) {
	// echo!!!
	// ZmqUtil::dump( frames );
	auto request = MajordomoWorkerCmd::Request::from( frames );
	if ( request.has_value() ) {
		MajordomoWorkerCmd::Reply reply{ .version = request.value().version,
			                             .clientAddr = request.value().clientAddr,
			                             .body = request.value().body };
		spdlog::info( "Send reply back to client: {}", reply.clientAddr );
		ZmqUtil::sendAllFrames( *mSocket, MajordomoWorkerCmd::Reply::to( reply ) );
	}
}

void MajordomoWorker::receive() {
	mIsRunning = true;
	while ( mIsRunning ) {
		auto evt = mPoller.wait( mSelfHeartbeatPeriod );
		if ( !evt ) {
			//timeout
			checkRemoteHeartbeat();
		}
		updateHeartbeat();
	}
}

void MajordomoWorker::checkRemoteHeartbeat() {
	if ( mMissingHeartbeat >= gMaxMissingHeartbeats ) {
		std::this_thread::sleep_for( std::chrono::milliseconds( 3000 ) );
		connect();
	} else {
		mMissingHeartbeat++;
	}
}

void MajordomoWorker::updateHeartbeat() {
	if ( std::chrono::steady_clock::now() > mHeartbeatDeadline ) {
		MajordomoWorkerCmd::Heartbeat heartbeat{ .version = gMajVer };
		if ( mSocket ) {
			ZmqUtil::sendAllFrames( *mSocket, MajordomoWorkerCmd::Heartbeat::to( heartbeat ) );
		}
		mHeartbeatDeadline = std::chrono::steady_clock::now() + mSelfHeartbeatPeriod;
	}
}