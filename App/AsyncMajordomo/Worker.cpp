#include <MajordomoWorker.h>
#include <iostream>
#include <spdlog/spdlog.h>

int main( int argc, char *argv[] ) {
	if ( argc != 2 ) {
		std::cerr << "Number of arguments must be 2" << std::endl;
		exit( EXIT_FAILURE );
	}
	auto service = std::string( argv[ 1 ] );
	spdlog::set_level( spdlog::level::debug );
	MajordomoWorker maj{ "tcp://localhost:5556", service, std::chrono::seconds( 1 ) };
	maj.receive();
	return 0;
}