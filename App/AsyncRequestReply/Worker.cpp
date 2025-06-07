#include <chrono>
#include <thread>

#include <spdlog/spdlog.h>
#include <zmq.hpp>

int main( int argc, char *argv[] ) {
	zmq::context_t ctx;
	zmq::socket_t worker{ ctx, zmq::socket_type::rep };
	worker.connect("tcp://localhost:5560");

	while ( 1 ) {
		// wait request from client
		zmq::message_t request;
		auto recvSize = worker.recv( request, zmq::recv_flags::none );
		if ( recvSize > 0 ) {
			spdlog::info( "Received request: {}", request.to_string() );
		}
		// do some work
		std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

		// reply
		zmq::message_t reply{ std::string_view{ "World" } };
		worker.send( reply, zmq::send_flags::none );
	}
}