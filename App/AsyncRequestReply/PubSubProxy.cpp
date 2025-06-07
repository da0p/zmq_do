#include <zmq.hpp>
#include <zmq_addon.hpp>

int main( int argc, char *argv[] ) {
	zmq::context_t ctx;

	// connect to the weather server in the internal network
	zmq::socket_t front{ ctx, zmq::socket_type::sub };
	front.connect( "tcp://localhost:5556" );

	// assume we set up another endpoint for subscribers
	zmq::socket_t back{ ctx, zmq::socket_type::pub };
	back.bind( "tcp://*:8100" );

	// subscribe on everything
	front.set( zmq::sockopt::subscribe, "" );

	zmq::active_poller_t poller;
	poller.add( front, zmq::event_flags::pollin, [ &front, &back ]( zmq::event_flags flag ) {
		zmq::message_t message;
		while ( 1 ) {
			[[maybe_unused]] auto _ = front.recv( message, zmq::recv_flags::none );
			auto more = front.get( zmq::sockopt::rcvmore );
			back.send( message, more ? zmq::send_flags::sndmore : zmq::send_flags::none );
			if ( !more ) {
				break;
			}
		}
	} );

	while ( 1 ) {
		[[maybe_unused]] auto _ = poller.wait( std::chrono::milliseconds( 100 ) );
	}

	return 0;
}