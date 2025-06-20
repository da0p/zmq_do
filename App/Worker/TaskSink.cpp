#include <chrono>

#include <ratio>
#include <spdlog/spdlog.h>
#include <zmq.hpp>

#include <StopTimer.h>

int main( int argc, char *argv[] ) {
	zmq::context_t zmqContext;
	zmq::socket_t zmqSink{ zmqContext, zmq::socket_type::pull };
	zmqSink.bind( "tcp://*:5558" );

	zmq::socket_t controller{ zmqContext, zmq::socket_type::pub };
	controller.bind( "tcp://*:5559" );

	// wait for start event
	StopTimer timer;
	zmq::message_t startEvent;
	[[maybe_unused]] auto _ = zmqSink.recv( startEvent, zmq::recv_flags::none );
	timer.start();

	// wait for 100 results
	for ( auto i = 0; i < 100; i++ ) {
		zmq::message_t message;
		auto result = zmqSink.recv( message, zmq::recv_flags::none );
		if ( result.has_value() ) {
			spdlog::info( "Receiving result {}", i );
		}
	}

	spdlog::info( "Total elasped time: {} ms", timer.getTimeElasped().count() );

	zmq::message_t stopMsg{ std::string_view( "STOP" ) };
	controller.send( stopMsg, zmq::send_flags::none );

	return 0;
}