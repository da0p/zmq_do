#include <format>
#include <iostream>

#include <FreelanceServer.h>

int main( int argc, char *argv[] ) {
	if ( argc != 2 ) {
		std::cout << std::format( "Usage:\n{} port", argv[ 0 ] );
		exit( EXIT_FAILURE );
	}

	auto port = std::string( argv[ 1 ] );
	FreelanceServer server{ port };
	server.run();

	return 0;
}