#include <FreelanceServer.h>

#include <spdlog/spdlog.h>

#include <RandomString.h>
#include <ZmqUtil.h>

FreelanceServer::FreelanceServer( const std::string &port ) : mSocket{ mZmqCtx, zmq::socket_type::router } {
	setup( port );
}

void FreelanceServer::setup( const std::string &port ) {
	auto serverId = std::format( "tcp://localhost:{}", port );
	mSocket.set( zmq::sockopt::routing_id, serverId );

	auto endpoint = std::format( "tcp://*:{}", port );
	mSocket.bind( endpoint );
	spdlog::info( "Service is ready at {}", endpoint );
}

void FreelanceServer::run() {
	zmq::active_poller_t poller;
	poller.add( mSocket, zmq::event_flags::pollin, [ this ]( zmq::event_flags flag ) { handleIncomingMessage(); } );

	while ( 1 ) {
		[[maybe_unused]] auto _ = poller.wait( std::chrono::milliseconds( 100 ) );
	}
}

void FreelanceServer::handleIncomingMessage() {
	zmq::multipart_t request;
	auto recv = zmq::recv_multipart( mSocket, std::back_inserter( request ) );
	if ( recv.has_value() ) {
		spdlog::info( "received request:\n" );

		ZmqUtil::dump( request, true );
		std::string clientId = request.popstr();
		std::string control = request.popstr();
		zmq::multipart_t reply;
		if ( control == "PING" ) {
			reply.addstr( "PONG" );
		} else {
			reply.addstr( control );
			reply.addstr( "OK" );
		}
		reply.pushstr( clientId );
		spdlog::info( "send reply to client={}\n", clientId );

		ZmqUtil::dump( reply, true );
		reply.send( mSocket );
	} else {
		spdlog::error( "Failed to receive messages!" );
	}
}