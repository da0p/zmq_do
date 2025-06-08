#include <spdlog/spdlog.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>

int main( int argc, char *argv[] ) {
	zmq::context_t ctx;
	// connect subscriber socket
	zmq::socket_t subscriber{ ctx, zmq::socket_type::sub };
	subscriber.connect( "tcp://localhost:5561" );
	subscriber.set( zmq::sockopt::subscribe, "" );

	// synchronize with publisher
	zmq::socket_t client{ ctx, zmq::socket_type::req };
	client.connect( "tcp://localhost:5562" );

	// send sync request
	zmq::message_t message{ std::string( "" ) };
	client.send( message, zmq::send_flags::none );
	[[maybe_unused]] auto _ = client.recv( message, zmq::recv_flags::none );
	spdlog::info( "Received sync!" );

	size_t updates{ 0 };
	bool isRunning{ true };
	zmq::active_poller_t poller;
	poller.add( subscriber, zmq::event_flags::pollin, [ &updates, &isRunning, &subscriber ]( zmq::event_flags flag ) {
		zmq::message_t message;
		auto size = subscriber.recv( message, zmq::recv_flags::none );
		if ( size > 0 ) {
			if ( !message.to_string().compare( "end" ) ) {
				isRunning = false;
			} else {
				updates++;
			}
		}
	} );

	while ( isRunning ) {
		[[maybe_unused]] auto _ = poller.wait( std::chrono::milliseconds( 100 ) );
	}
	spdlog::info( "Received {} updates!", updates );

	return 0;
}