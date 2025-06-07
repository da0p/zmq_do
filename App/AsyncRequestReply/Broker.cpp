#include <zmq.hpp>
#include <zmq_addon.hpp>

int main( int argc, char *argv[] ) {
	zmq::context_t ctx;
	zmq::socket_t front{ ctx, zmq::socket_type::router };
	zmq::socket_t back{ ctx, zmq::socket_type::dealer };

	front.bind( "tcp://*:5559" );
	back.bind( "tcp://*:5560" );

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

	poller.add( back, zmq::event_flags::pollin, [ &front, &back ]( zmq::event_flags flag ) {
		zmq::message_t message;
		while ( 1 ) {
			[[maybe_unused]] auto _ = back.recv( message, zmq::recv_flags::none );
			auto more = back.get( zmq::sockopt::rcvmore );
			front.send( message, more ? zmq::send_flags::sndmore : zmq::send_flags::none );
			if ( !more ) {
				break;
			}
		}
	} );

	std::vector<zmq::poller_event<>> events( 2 );
	while ( 1 ) {
		[[maybe_unused]] auto _ = poller.wait( std::chrono::milliseconds( 100 ) );
	}

	return 0;
}