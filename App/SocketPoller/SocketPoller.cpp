#include <chrono>
#include <spdlog/spdlog.h>

#include <zmq.hpp>
#include <zmq_addon.hpp>

int main( int argc, char *argv[] ) {
	zmq::context_t zmqContext;

	// connect to task ventilator
	zmq::socket_t zmqPull{ zmqContext, zmq::socket_type::pull };
	zmqPull.connect( "tcp://localhost:5557" );

	// connect to weather server
	zmq::socket_t zmqSub{ zmqContext, zmq::socket_type::sub };
	zmqSub.connect( "tcp://localhost:5556" );
	zmqSub.set( zmq::sockopt::subscribe, "10001" );

	zmq::active_poller_t poller;
	poller.add( zmqPull, zmq::event_flags::pollin, [ &zmqPull ]( zmq::event_flags flag ) {
		zmq::message_t message;
		auto msgSize = zmqPull.recv( message, zmq::recv_flags::dontwait );
		if ( msgSize.has_value() ) {
			std::string workLoad( static_cast<char *>( message.data() ), msgSize.value() );
			spdlog::info( "Received workLoad = {} ms", workLoad );
		}
	} );

	poller.add( zmqSub, zmq::event_flags::pollin, [ &zmqSub ]( zmq::event_flags flag ) {
		zmq::message_t message;
		auto msgSize = zmqSub.recv( message, zmq::recv_flags::dontwait );
		if ( msgSize.has_value() ) {
			uint32_t zipCode{ 0 };
			float temperature{ 0.0F };
			float humidity{ 0.0F };
			std::istringstream iStream{ static_cast<char *>( message.data() ) };
			iStream >> zipCode >> temperature >> humidity;
			spdlog::info( "Zipcode = {}, Temperature = {}, Humidity = {}", zipCode, temperature, humidity );
		}
	} );
	std::vector<zmq::poller_event<>> events( poller.size() );
	while ( 1 ) {
		[[maybe_unused]] auto _ = poller.wait( std::chrono::milliseconds( 100 ) );
	}

	return 0;
}