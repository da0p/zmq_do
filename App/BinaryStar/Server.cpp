#include "BinaryStarServer.h"

#include <cstdlib>
#include <exception>
#include <iostream>

int main( int argc, char *argv[] ) {
	if ( argc != 2 ) {
		std::cerr << "Number of arguments must be 2" << std::endl;
		exit( EXIT_FAILURE );
	}

	bool primary{ true };
	try {
		primary = static_cast<bool>( std::stoi( argv[ 1 ] ) );
	} catch ( std::exception &e ) {
		std::cerr << "Argument must be 0 or 1" << std::endl;
		exit( EXIT_FAILURE );
	}

	if ( primary ) {
		BinaryStarServer server{ primary, "tcp://*:5001", "tcp://*:5003", "tcp://localhost:5004" };
		server.run();
	} else {
		BinaryStarServer server{ primary, "tcp://*:5002", "tcp://*:5004", "tcp://localhost:5003" };
		server.run();
	}

	return 0;
}