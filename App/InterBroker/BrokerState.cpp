#include <BrokerState.h>

#include <string>
#include <string_view>

#include <spdlog/spdlog.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <RandomNumberGenerator.h>
#include <ZmqUtil.h>

BrokerState::BrokerState( zmq::context_t &ctx ) :
        mStateBackend{ ctx, zmq::socket_type::pub },
        mStateFrontend{ ctx, zmq::socket_type::sub } {
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

void BrokerState::broadcastMyState() {
	RandomNumberGenerator rnd{ 0, 10 };
	ZmqUtil::sendString( mStateBackend, mMyId, zmq::send_flags::sndmore );
	ZmqUtil::sendString( mStateBackend, std::to_string( rnd.generate() ) );
	spdlog::info( "Broadcast my state...\n" );
}

void BrokerState::run() {
	zmq::active_poller_t poller;
	poller.add( mStateFrontend, zmq::event_flags::pollin, [ this ]( zmq::event_flags flag ) {
		auto peerName = ZmqUtil::recvString( mStateFrontend );
		auto idleWorkers = ZmqUtil::recvString( mStateFrontend );
		if ( peerName.has_value() && idleWorkers.has_value() ) {
			auto newIdleWorkers = std::stoi( idleWorkers.value() );
			if ( mOtherIdleWorkers.contains( peerName.value() ) ) {
				spdlog::info( "Updating [{}, {}] -> [{}, {}]",
				              peerName.value(),
				              mOtherIdleWorkers.at( peerName.value() ),
				              peerName.value(),
				              newIdleWorkers );
			} else {
				spdlog::info( "New subscription from [{}, {}]",
				              peerName.value(),
				              newIdleWorkers );
			}
			mOtherIdleWorkers[ peerName.value() ] = newIdleWorkers;
		}
	} );

	while ( 1 ) {
		if ( poller.wait( std::chrono::seconds( 1 ) ) == 0 ) {
			broadcastMyState();
		}
	}
}