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
		maj.send( service, MajordomoClientMessage::from( "HelloWorld" ) );
	}

	size_t i = 0;
	for ( i = 0; i < 100000; i++ ) {
		auto reply = maj.recv( std::chrono::milliseconds( 2500 ) );
		if ( !reply.has_value() ) {
			spdlog::warn( "empty reply" );
			break;
		}
	}
	spdlog::critical( "stop at i = {}, elapsed time: {} ms", i, timer.getTimeElasped().count() );
	return 0;
}