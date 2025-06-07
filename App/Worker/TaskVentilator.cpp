#include <iostream>

#include <spdlog/spdlog.h>
#include <zmq.hpp>

#include <RandomNumberGenerator.h>

namespace {
	void sendStart( zmq::context_t &context ) {
		// Notify starting
		zmq::socket_t zmqSink{ context, zmq::socket_type::push };
		zmqSink.connect( "tcp://localhost:5558" );
		zmq::message_t message( 2 );
		std::memcpy( message.data(), "0", 1 );
		zmqSink.send( message, zmq::send_flags::none );
	}
}

int main( int argc, char *argv[] ) {
	zmq::context_t zmqContext{ 1 };
	// Interesting, in push-pull model, this is a fixed address. This will help
	// to scale the number of workers
	zmq::socket_t zmqSender{ zmqContext, zmq::socket_type::push };
	zmqSender.bind( "tcp://*:5557" );

	spdlog::info( "Press Enter when all workers are ready" );
	// Wait here
	std::cin.get();

	auto rdGen = RandomNumberGenerator( 1, 100 );
	spdlog::info( "Sending tasks to workers...\n" );
	sendStart( zmqContext );

	int32_t totalMillSeconds = 0;
	for ( auto i = 0; i < 100; i++ ) {
		auto workLoad = rdGen.generate();
		totalMillSeconds += workLoad;

		std::string request;
		std::format_to( std::back_inserter( request ), "{}", workLoad );
		zmq::message_t message{ request.length() };
		std::memcpy( message.data(), request.data(), request.length() );
		spdlog::info( "Sending workLoad = {} ms", request );
		zmqSender.send( message, zmq::send_flags::none );
		request.clear();
	}

	spdlog::info( "Total expected cost: {} milliseconds", totalMillSeconds );
	std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

	return 0;
}
