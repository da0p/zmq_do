#include <spdlog/spdlog.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <ZmqUtil.h>

int main( int argc, char *argv[] ) {
	zmq::context_t ctx;

	zmq::socket_t router{ ctx, zmq::socket_type::router };
	router.bind( "inproc://server" );

	// let zmq set the identity
	zmq::socket_t anonymous{ ctx, zmq::socket_type::req };
	anonymous.connect( "inproc://server" );
	ZmqUtil::sendString( anonymous, "ROUTER uses a generated 5 byte identity" );
	ZmqUtil::dump( router );

	// set the identity ourselves
	zmq::socket_t identified{ ctx, zmq::socket_type::req };
	identified.set( zmq::sockopt::routing_id, "PEERR" );
	// Interesting here: even though the generated zmq identity contains only 5 bytes,
	// we can use more than 5 bytes to set identity
	identified.connect( "inproc://server" );
	ZmqUtil::sendString( identified, "ROUTER socket uses REQ's socket identity" );
	ZmqUtil::dump( router );

	return 0;
}