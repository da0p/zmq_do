#include <thread>

#include <spdlog/spdlog.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <RandomNumberGenerator.h>
#include <ZmqUtil.h>

int main( int argc, char *argv[] ) {

	zmq::context_t ctx;
	zmq::socket_t server{ ctx, zmq::socket_type::rep };
	server.bind( "tcp://*:5555" );

	RandomNumberGenerator rnd{ 0, 3 };
	uint32_t cycles{ 0 };
	while ( 1 ) {
		auto request = ZmqUtil::recvString( server );
		cycles++;

		if ( cycles > 5 && rnd.generate() == 0 ) {
			spdlog::info( "Simulate a crash and then run again\n" );
			cycles = 0;
			break;
		} else if ( cycles > 5 && rnd.generate() == 0 ) {
			spdlog::info( "Simulate CPU overload\n" );
			std::this_thread::sleep_for( std::chrono::seconds( 2 ) );
			cycles = 0;
		}

		if ( request.has_value() ) {
			spdlog::info( "Normal request ({})\n", request.value() );
			std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
			ZmqUtil::sendString( server, request.value() );
		}
	}

	return 0;
}