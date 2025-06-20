#include "Common.h"
#include <MajordomoClient.h>
#include <StopTimer.h>
#include <chrono>
#include <spdlog/spdlog.h>

int main( int argc, char *argv[] ) {
	spdlog::set_level( spdlog::level::err );
	MajordomoClient maj{ "tcp://localhost:5555" };
	StopTimer timer;
	timer.start();
	for ( size_t i = 0; i < 100000; i++ ) {
		auto reply = maj.send( "echo", MajordomoClientCmd::from( "HelloWorld" ), std::chrono::milliseconds( 2500 ), 3 );
		if ( !reply.has_value() ) {
			break;
		}
	}
	spdlog::critical( "elapsed time: {} ms", timer.getTimeElasped().count() );
	return 0;
}