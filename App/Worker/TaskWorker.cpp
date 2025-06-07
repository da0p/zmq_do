#include <chrono>
#include <thread>

#include <spdlog/spdlog.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>

int main( int argc, char *argv[] ) {
	zmq::context_t zmqContext;

	// socket to receive messages on
	zmq::socket_t zmqPull{ zmqContext, zmq::socket_type::pull };
	zmqPull.connect( "tcp://localhost:5557" );

	// socket to send messages to
	zmq::socket_t zmqPush{ zmqContext, zmq::socket_type::push };
	zmqPush.connect( "tcp://localhost:5558" );

	zmq::socket_t controller{ zmqContext, zmq::socket_type::sub };
	controller.connect( "tcp://localhost:5559" );
	controller.set( zmq::sockopt::subscribe, "STOP" );

	bool isRunning{ true };

	zmq::active_poller_t poller;
	poller.add( zmqPull, zmq::event_flags::pollin, [ &zmqPull, &zmqPush ]( zmq::event_flags flag ) {
		zmq::message_t message;
		auto size = zmqPull.recv( message, zmq::recv_flags::none );
		if ( size.has_value() ) {
			auto workLoad = std::stoi( message.to_string() );
			spdlog::info( "Receiving workLoad = {} ms", workLoad );
			std::this_thread::sleep_for( std::chrono::milliseconds( workLoad ) );
			message.rebuild();
			zmqPush.send( message, zmq::send_flags::none );
		}
	} );

	poller.add( controller, zmq::event_flags::pollin, [ &isRunning ]( zmq::event_flags flag ) { isRunning = false; } );

	while ( isRunning ) {
		[[maybe_unused]] auto _ = poller.wait( std::chrono::milliseconds( 100 ) );
	}

	return 0;
}