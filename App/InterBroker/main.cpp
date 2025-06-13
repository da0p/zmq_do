#include <iostream>
#include <string>
#include <thread>

#include <spdlog/spdlog.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <ZmqUtil.h>

#include <Client.h>
#include <WorkRouter.h>
#include <Worker.h>

namespace {
	constexpr size_t gNumberOfClients{ 3 };
	constexpr size_t gNumberOfWorkers{ 3 };

	void usageError( std::string_view prog, std::string_view msg ) {
		std::cerr << msg << "\n";
		std::cout << std::format( "Usage: {} me broker1 ...", prog );
		exit( EXIT_FAILURE );
	}
}

int main( int argc, char *argv[] ) {

	if ( argc < 2 ) {
		usageError( argv[ 0 ], "" );
	}

	std::string myId = argv[ 1 ];
	std::vector<std::string> peers;
	for ( size_t i = 2; i < static_cast<size_t>( argc ); i++ ) {
		peers.push_back( argv[ i ] );
	}

	zmq::context_t ctx;
	// start local clients
	std::vector<std::unique_ptr<Client>> clients;
	for ( size_t i = 0; i < gNumberOfClients; i++ ) {
		clients.push_back( std::make_unique<Client>( std::ref( ctx ), myId, static_cast<uint32_t>( i ) ) );
		clients[ i ]->run();
	}

	// start local workers
	std::vector<std::unique_ptr<Worker>> workers;
	for ( size_t i = 0; i < gNumberOfWorkers; i++ ) {
		workers.push_back( std::make_unique<Worker>( std::ref( ctx ), myId, static_cast<uint32_t>( i ) ) );
		workers[ i ]->run();
	}

	spdlog::info( "Press enter when all brokers are started!\n" );
	std::cin.get();

	spdlog::info( "Start workRouter" );
	WorkRouter workRouter{ ctx };
	workRouter.setupSelf( myId );
	workRouter.setupRemote( peers );
	workRouter.run();

	for ( size_t i = 0; i < gNumberOfClients; i++ ) {
		clients[ i ]->stop();
	}

	for ( size_t i = 0; i < gNumberOfWorkers; i++ ) {
		workers[ i ]->stop();
	}

	return 0;
}