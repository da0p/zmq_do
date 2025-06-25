#include "HeartbeatService.h"

#include <spdlog/spdlog.h>

#include <thread>

HeartbeatService::HeartbeatService( zmq::context_t &ctx, const std::string &endpoint, std::chrono::milliseconds hbInterval ) :
        mSocket{ ctx, zmq::socket_type::pair }, mHeartbeatInterval{ hbInterval } {
	setup( endpoint );
}

void HeartbeatService::setup( const std::string &endpoint ) {
	mSocket.connect( endpoint );
}

void HeartbeatService::run() {
	mDeadline = std::chrono::steady_clock::now();
	while ( 1 ) {
		sendHeartbeat();
		std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
	}
}

void HeartbeatService::sendHeartbeat() {
	if ( std::chrono::steady_clock::now() > mDeadline ) {
		spdlog::debug( "sending heartbeat..." );
		zmq::message_t ping{ std::string( "PING" ) };
		mSocket.send( ping, zmq::send_flags::none );
		mDeadline = std::chrono::steady_clock::now() + mHeartbeatInterval;
	}
}