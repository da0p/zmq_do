#include <thread>

#include <spdlog/spdlog.h>
#include <zmq.hpp>

#include <RandomNumberGenerator.h>

int main( int argc, char *argv[] ) {

	zmq::context_t zmqContex{ 1 };
	zmq::socket_t publisher{ zmqContex, zmq::socket_type::pub };
	publisher.bind( "tcp://*:5556" );
	publisher.bind( "ipc://weather.ipc" );
	auto random = RandomNumberGenerator( 10000, 10010 );

	while ( 1 ) {
		auto zipCode = std::format( "{:{}d}", random.generate(), 5 );
		auto temperature = std::format( "{:{}.{}f}", static_cast<float>( random.generate() ) / 1000, 3, 1 );
		auto humidity = std::format( "{:{}.{}f}", static_cast<float>( random.generate() ) / 1000, 3, 1 );
		std::string text;
		std::format_to( std::back_inserter( text ), "{} {} {}", zipCode, temperature, humidity );
		zmq::message_t notification{ text.length() };
		std::memcpy( notification.data(), text.data(), text.length() );
		spdlog::info( "Publish weather update. Zip code = {}, temperature = {}, humidity = {}", zipCode, temperature, humidity );
		publisher.send( notification, zmq::send_flags::none );
		std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
	}

	return 0;
}