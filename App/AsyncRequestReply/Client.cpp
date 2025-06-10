#include <spdlog/spdlog.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <ZmqUtil.h>

int main( int argc, char *argv[] ) {
	zmq::context_t zmqContext;
	zmq::socket_t zmqRequester{ zmqContext, zmq::socket_type::req };
	zmqRequester.connect( "tcp://localhost:5559" );

	for ( auto i = 0; i < 10; i++ ) {
		ZmqUtil::sendString( zmqRequester, "Hello" );
		auto reply = ZmqUtil::recvString( zmqRequester );
		spdlog::info( "Received reply: {}", reply.value() );
	}

	return 0;
}