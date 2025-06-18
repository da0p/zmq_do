#include "Common.h"
#include <MajordomoClient.h>
#include <chrono>

int main( int argc, char *argv[] ) {
	MajordomoClient maj{ "tcp://localhost:5555" };

	for ( size_t i = 0; i < 100000; i++ ) {
		auto reply = maj.send( "echo", fromStr( "HelloWorld" ), std::chrono::milliseconds( 2500 ), 3 );
		if ( !reply.has_value() ) {
			break;
		}
	}
	return 0;
}