#include "ZmqUtil.h"
#include <WorkRouter.h>

#include <queue>

#include <spdlog/spdlog.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>

WorkRouter::WorkRouter( zmq::context_t &ctx ) :
        mLocalFrontendSocket{ ctx, zmq::socket_type::router },
        mLocalBackendSocket{ ctx, zmq::socket_type::router },
        mRemoteFrontendSocket{ ctx, zmq::socket_type::router },
        mRemoteBackendSocket{ ctx, zmq::socket_type::router },
        mBrokerState{ ctx } {
}

void WorkRouter::setupSelf( const std::string &myId ) {
	mMyId = myId;

	mBrokerState.setupSelf( myId );

	auto frontUrl = std::format( "ipc://{}-localfe.ipc", myId );
	mLocalFrontendSocket.bind( frontUrl );

	auto backUrl = std::format( "ipc://{}-localbe.ipc", myId );
	mLocalBackendSocket.bind( backUrl );
}

void WorkRouter::setupRemote( const std::vector<std::string> &peers ) {
	mBrokerState.setupPeers( peers );

	mRemoteFrontendSocket.set( zmq::sockopt::routing_id, mMyId );
	auto frontUrl = std::format( "ipc://{}-remote.ipc", mMyId );
	mRemoteFrontendSocket.bind( frontUrl );

	mRemoteBackendSocket.set( zmq::sockopt::routing_id, mMyId );
	for ( const auto &peer : peers ) {
		spdlog::info( "Connect to remote frontend at {}\n", peer );
		auto backUrl = std::format( "ipc://{}-remote.ipc", peer );
		mRemoteBackendSocket.connect( backUrl );
	}
}

void WorkRouter::run() {
	zmq::active_poller_t poller;
	poller.add(
	  mLocalBackendSocket, zmq::event_flags::pollin, [ this ]( zmq::event_flags flag ) { handleLocalBackend(); } );
	poller.add(
	  mRemoteBackendSocket, zmq::event_flags::pollin, [ this ]( zmq::event_flags flag ) { handleRemoteBackend(); } );
	mBrokerState.add2Poller( poller );
	mIsRunning = true;
	while ( mIsRunning ) {
		try {
			handleCapacity( poller );
			poller.wait( std::chrono::seconds( 1 ) );
			mBrokerState.broadcastMyState();
		} catch ( zmq::error_t &e ) {
			mIsRunning = false;
		}
	}
}

void WorkRouter::handleCapacity( zmq::active_poller_t &poller ) {
	if ( mBrokerState.isLocalWorkerAvailable() && !mIsPollingFront ) {
		mIsPollingFront = true;
		poller.add(
		  mLocalFrontendSocket, zmq::event_flags::pollin, [ this ]( zmq::event_flags flag ) { handleLocalFrontend(); } );
		poller.add( mRemoteFrontendSocket, zmq::event_flags::pollin, [ this ]( zmq::event_flags flag ) {
			handleRemoteFrontend();
		} );
	} else if ( !mBrokerState.isLocalWorkerAvailable() && mIsPollingFront ) {
		poller.remove( mLocalFrontendSocket );
		poller.remove( mRemoteFrontendSocket );
		mIsPollingFront = false;
	}
}

void WorkRouter::handleLocalFrontend() {
	auto rawFrames = ZmqUtil::recvAllStrings( mLocalFrontendSocket );
	if ( !rawFrames.has_value() ) {
		return;
	}

	// use local workers
	if ( mBrokerState.isLocalWorkerAvailable() ) {
		auto worker = mBrokerState.getLocalWorker();
		std::vector<std::string> sendFrames;
		sendFrames.push_back( worker );
		sendFrames.push_back( "" );
		std::copy( rawFrames.value().begin(), rawFrames.value().end(), std::back_inserter( sendFrames ) );
		spdlog::info( "Send to local worker: {}", worker );
		ZmqUtil::sendAllStrings( mLocalBackendSocket, sendFrames );
	} else if ( mBrokerState.isRemotePeerAvailable() ) {
		auto peer = mBrokerState.getRemotePeer();
		std::vector<std::string> sendFrames;
		sendFrames.push_back( peer );
		sendFrames.push_back( "" );
		std::copy( rawFrames.value().begin(), rawFrames.value().end(), std::back_inserter( sendFrames ) );
		spdlog::info( "Send to remote peer: {}", peer );
		ZmqUtil::sendAllStrings( mRemoteBackendSocket, sendFrames );
	} else {
		spdlog::warn( "No idle worker!!!" );
	}
}

void WorkRouter::handleRemoteFrontend() {
	auto rawFrames = ZmqUtil::recvAllStrings( mRemoteFrontendSocket );
	if ( !rawFrames.has_value() ) {
		return;
	}

	if ( !mBrokerState.isLocalWorkerAvailable() ) {
		spdlog::error( "handleRemoteFrontend - no idle workers" );
		return;
	}
	auto worker = mBrokerState.getLocalWorker();
	std::vector<std::string> sendFrames;
	sendFrames.push_back( worker );
	sendFrames.push_back( "" );
	std::copy( rawFrames.value().begin(), rawFrames.value().end(), std::back_inserter( sendFrames ) );
	ZmqUtil::sendAllStrings( mLocalBackendSocket, sendFrames );
}

void WorkRouter::handleLocalBackend() {
	auto workerId = ZmqUtil::recvString( mLocalBackendSocket );
	if ( !workerId.has_value() ) {
		return;
	}
	mBrokerState.addIdleWorker( workerId.value() );
	[[maybe_unused]] auto emptyDelimiter = ZmqUtil::recvString( mLocalBackendSocket );

	auto rawFrames = ZmqUtil::recvAllStrings( mLocalBackendSocket );
	if ( !rawFrames.has_value() ) {
		return;
	}

	auto allFrames = rawFrames.value();
	// if the next frame is `ready`, we can just go on
	if ( allFrames.size() == 1 && allFrames.front() == "ready" ) {
		spdlog::info( "worker {} sends `ready`", workerId.value() );
		return;
	} else if ( allFrames.size() == 3 ) {
		// if the received frames size is 3, it means the original request comes
		// from a local client
		// [client addr][0][ok]
		// then send the reply to the local client
		ZmqUtil::sendAllStrings( mLocalFrontendSocket, allFrames );
	} else if ( allFrames.size() == 5 ) {
		// if the received frames size is 5, it means the original request comes
		// from a remote client
		// [remote broker][0][client addr][0][ok]
		ZmqUtil::sendAllStrings( mRemoteFrontendSocket, allFrames );
	}
}

void WorkRouter::handleRemoteBackend() {
	auto rawFrames = ZmqUtil::recvAllStrings( mRemoteBackendSocket );
	if ( !rawFrames.has_value() ) {
		return;
	}
	auto allFrames = rawFrames.value();
	if ( allFrames.size() != 5 ) {
		spdlog::error( "Can't route reply. Reply's size must be 5" );
		return;
	}

	spdlog::info( "received request from peer: {}", allFrames.front() );
	std::vector<std::string> reply( allFrames.begin() + 2, allFrames.end() );
	ZmqUtil::sendAllStrings( mLocalFrontendSocket, reply );
}