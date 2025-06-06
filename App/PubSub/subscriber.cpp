#include <sstream>

#include <spdlog/spdlog.h>
#include <zmq.hpp>

int main( int argc, char *argv[] ) {
	zmq::context_t zmqContext;
	spdlog::info( "Collecting updates from weather server...\n" );
	zmq::socket_t subscriber{ zmqContext, zmq::socket_type::sub };
	subscriber.connect( "tcp://localhost:5556" );

	// Filter only for zipCode = 10001. This step is always needed. Otherwise we
	// will receive nothing
	subscriber.set( zmq::sockopt::subscribe, "10001");

    // Try to set another subscription
    subscriber.set(zmq::sockopt::subscribe, "10002");

	while ( 1 ) {
		uint32_t zipCode;
		float temperature;
		float humidity;
		zmq::message_t update;
		auto result = subscriber.recv( update, zmq::recv_flags::none );
		if ( result.has_value() ) {
			std::istringstream iStream{ static_cast<char *>( update.data() ) };
			iStream >> zipCode >> temperature >> humidity;
			spdlog::info( "Zipcode = {}, Temperature = {}, Humidity = {}", zipCode, temperature, humidity );
		}
	}

	return 0;
}