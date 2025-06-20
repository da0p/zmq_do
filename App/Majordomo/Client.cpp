#include "Common.h"
#include <MajordomoClient.h>
#include <StopTimer.h>
#include <chrono>
#include <iostream>
#include <spdlog/spdlog.h>

int main( int argc, char *argv[] ) {
	if ( argc != 2 ) {
		std::cerr << "Number of arguments must be 2" << std::endl;
		exit( EXIT_FAILURE );
	}

	auto service = std::string( argv[ 1 ] );

	spdlog::set_level( spdlog::level::debug );
	MajordomoClient maj{ "tcp://localhost:5555" };
	StopTimer timer;
	timer.start();
	for ( size_t i = 0; i < 100000; i++ ) {
		auto reply = maj.send( service, MajordomoClientCmd::from( "HelloWorld" ), std::chrono::milliseconds( 2500 ), 3 );
		if ( !reply.has_value() ) {
			break;
		}
	}
	spdlog::critical( "elapsed time: {} ms", timer.getTimeElasped().count() );
	return 0;
}