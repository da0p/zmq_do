#include <iostream>
#include <thread>
#include <vector>

#include <FreelanceClientProxy.h>
#include <HeartbeatService.h>
#include <RequestService.h>

int main( int argc, char *argv[] ) {

	if ( argc < 2 ) {
		std::cout << std::format( "Usage:\n{} port-1 port-2", argv[ 0 ] );
		exit( EXIT_FAILURE );
	}

	std::vector<uint16_t> ports;
	for ( auto i = 1; i < argc; i++ ) {
		ports.push_back( static_cast<uint16_t>( std::stoi( argv[ i ] ) ) );
	}

	zmq::context_t ctx;
	std::vector<std::thread> services;

	HeartbeatService heartbeat{ ctx, "inproc://heartbeat", std::chrono::milliseconds( 1000 ) };
	services.emplace_back( [ &heartbeat ] { heartbeat.run(); } );

	FreelanceProxy proxy{ ctx, ports, "inproc://heartbeat", "inproc://request" };
	services.emplace_back( [ &proxy ] { proxy.run(); } );

	RequestService request{ ctx, "inproc://request", std::chrono::milliseconds( 1000 ) };
	services.emplace_back( [ &request ] { request.run(); } );

	for ( size_t i = 0; i < services.size(); i++ ) {
		services[ i ].join();
	}

	return 0;
}