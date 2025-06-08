#include <chrono>
#include <iostream>
#include <thread>

#include <cstdlib>
#include <spdlog/spdlog.h>
#include <system_error>
#include <zmq.hpp>
#include <zmq_addon.hpp>

int main( int argc, char *argv[] ) {

	if ( argc != 2 ) {
		std::cerr << "Wrong number of arguments!" << std::endl;
		exit( EXIT_FAILURE );
	}

	auto numSubscribers = 0;
	try {
		numSubscribers = std::stoi( argv[ 1 ] );
	} catch ( const std::error_code &e ) {
		std::cerr << "Second argument must be a number!" << std::endl;
		exit( EXIT_FAILURE );
	}

	zmq::context_t ctx;
	zmq::socket_t publisher{ ctx, zmq::socket_type::pub };
	publisher.bind( "tcp://*:5561" );
	publisher.set( zmq::sockopt::sndhwm, 0 );
	// Do not silently drop messages
	publisher.set( zmq::sockopt::xpub_nodrop, 1 );

	zmq::socket_t receiver{ ctx, zmq::socket_type::rep };
	receiver.bind( "tcp://*:5562" );

	for ( auto i = 0; i < numSubscribers; i++ ) {
		zmq::message_t message;
		[[maybe_unused]] auto _ = receiver.recv( message, zmq::recv_flags::none );
		// send reply
		message.rebuild( "" );
		receiver.send( message, zmq::send_flags::none );
	}

	// all subscribers are ready, let's send 10M updates
	// we can see that there is a limit around ~4.1M
	spdlog::info( "Start sending updates!" );
	for ( size_t i = 0; i < 10000000; i++ ) {
		zmq::message_t message{ std::string( "ZeroMQ" ) };
		publisher.send( message, zmq::send_flags::none );
	}

	// allow zmq to flush output
	std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

	// signal the end
	zmq::message_t message{ std::string( "end" ) };
	publisher.send( message, zmq::send_flags::none );

	return 0;
}