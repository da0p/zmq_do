#include <BrokerState.h>

#include <string>
#include <string_view>

#include <spdlog/spdlog.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <RandomNumberGenerator.h>
#include <ZmqUtil.h>

BrokerState::BrokerState( zmq::context_t &ctx ) :
        mStateBackend{ ctx, zmq::socket_type::pub }, mStateFrontend{ ctx, zmq::socket_type::sub } {
}

void BrokerState::setupSelf( std::string_view name ) {
	mMyId = name;
	auto url = std::string( "ipc://" ) + std::string( name ) + "-state.ipc";
	mStateBackend.bind( url );
}

void BrokerState::setupPeers( const std::vector<std::string> &peerNames ) {
	mStateFrontend.set( zmq::sockopt::subscribe, "" );
	for ( const auto &name : peerNames ) {
		auto url = std::string( "ipc://" ) + name + "-state.ipc";
		mStateFrontend.connect( url );
	}
}

void BrokerState::addIdleWorker( std::string_view name ) {
	mMyIdleWorkers.push( std::string( name ) );
}

bool BrokerState::isLocalWorkerAvailable() {
	return !mMyIdleWorkers.empty();
}

std::string BrokerState::getLocalWorker() {
	auto idleWorker = mMyIdleWorkers.front();
	mMyIdleWorkers.pop();
	return idleWorker;
}

bool BrokerState::isRemotePeerAvailable() {
	return !mIdlePeers.empty();
}

std::string BrokerState::getRemotePeer() {
	// always take the first one
	auto it = mIdlePeers.begin();
	auto idleWorker = it->first;
	mIdlePeers.erase( it );
	return idleWorker;
}

void BrokerState::broadcastMyState() {
	ZmqUtil::sendString( mStateBackend, mMyId, zmq::send_flags::sndmore );
	ZmqUtil::sendString( mStateBackend, std::to_string( mMyIdleWorkers.size() ) );
}

void BrokerState::add2Poller( zmq::active_poller_t &poller ) {
	poller.add( mStateFrontend, zmq::event_flags::pollin, [ this ]( zmq::event_flags flag ) { handleStateUpdate(); } );
}

void BrokerState::handleStateUpdate() {
	auto peerName = ZmqUtil::recvString( mStateFrontend );
	auto idleWorkers = ZmqUtil::recvString( mStateFrontend );
	if ( peerName.has_value() && idleWorkers.has_value() ) {
		auto newIdleWorkers = std::stoi( idleWorkers.value() );
		if ( mIdlePeers.contains( peerName.value() ) ) {
			spdlog::debug(
			  "Updating [{}, {}] -> [{}, {}]", peerName.value(), mIdlePeers.at( peerName.value() ), peerName.value(), newIdleWorkers );
			if ( newIdleWorkers != 0 ) {
				mIdlePeers[ peerName.value() ] = newIdleWorkers;
			} else {
				mIdlePeers.erase( peerName.value() );
			}
		} else {
			spdlog::debug( "New subscription from [{}, {}]", peerName.value(), newIdleWorkers );
			if ( newIdleWorkers != 0 ) {
				mIdlePeers[ peerName.value() ] = newIdleWorkers;
			}
		}
	}
}