#include <iostream>

#include <string>
#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <ZmqUtil.h>

#include <BrokerState.h>

void usageError( std::string_view prog, std::string_view msg ) {
	std::cerr << msg << "\n";
	std::cout << std::format( "Usage: {} me broker1 ...", prog );
	exit( EXIT_FAILURE );
}

int main( int argc, char *argv[] ) {

	if ( argc < 2 ) {
		usageError( argv[ 0 ], "" );
	}

	std::string myId = argv[ 1 ];
	std::vector<std::string> otherIds;
	for ( size_t i = 2; i < static_cast<size_t>( argc ); i++ ) {
		otherIds.push_back( argv[ i ] );
	}

	zmq::context_t ctx;
	BrokerState brokerState{ ctx };
	brokerState.setupSelf( myId );
	brokerState.setupPeers( otherIds );
	brokerState.run();

	return 0;
}