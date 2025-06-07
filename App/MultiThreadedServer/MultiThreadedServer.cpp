#include <iostream>
#include <sstream>
#include <thread>

#include <spdlog/spdlog.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>

void doWork( zmq::context_t &ctx ) {
	zmq::socket_t socket{ ctx, zmq::socket_type::rep };
	socket.connect( "inproc://workers" );

	while ( 1 ) {
		zmq::message_t message;
		auto size = socket.recv( message, zmq::recv_flags::none );
		if ( size.has_value() ) {
			spdlog::info( "Received request: {}", message.to_string() );
			message.rebuild( std::string( "World" ) );
			std::ostringstream threadId;
			threadId << std::this_thread::get_id();
			spdlog::info( "Sending 'World' reply from threadId = {}", threadId.str() );
			socket.send( message, zmq::send_flags::none );
		}
	}
}

int main( int argc, char *argv[] ) {
	zmq::context_t ctx;
	zmq::socket_t front{ ctx, zmq::socket_type::router };
	front.bind( "tcp://localhost:5555" );

	// connect between threads using inproc socket is the safest way in zmq application
	zmq::socket_t back{ ctx, zmq::socket_type::dealer };
	back.bind( "inproc://workers" );

	std::vector<std::thread> workers;
	for ( auto i = 0; i < 5; i++ ) {
		workers.emplace_back( [ &ctx ] { doWork( ctx ); } );
	}
	zmq::proxy( front, back );

	for ( auto &worker : workers ) {
		worker.join();
	}

	return 0;
}