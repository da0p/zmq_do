#include <chrono>
#include <csignal>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>

#include <spdlog/spdlog.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <RandomNumberGenerator.h>
#include <ZmqUtil.h>

static volatile bool mIsRunning{ true };

static void handler( int sig ) {
	if ( sig == SIGINT ) {
		mIsRunning = false;
	}
}

void clientThread( zmq::context_t &ctx, uint32_t id ) {
	zmq::socket_t client{ ctx, zmq::socket_type::dealer };
	client.set( zmq::sockopt::routing_id, std::format( "client-{}", id ) );
	client.connect( "tcp://localhost:5570" );

	uint32_t requestId{ 0 };
	zmq::active_poller_t poller;
	poller.add( client, zmq::event_flags::pollin, [ &client, &id ]( zmq::event_flags flag ) {
		spdlog::info( "client id: {} received reply", id );
		ZmqUtil::dump( client );
	} );
	while ( mIsRunning ) {
		try {
			[[maybe_unused]] auto _ = poller.wait( std::chrono::seconds( 1 ) );
			ZmqUtil::sendString( client, std::format( "request #{}", requestId ) );
			requestId++;
		} catch ( zmq::error_t &e ) {
			spdlog::error( "client {} exits!", id );
		}
	}
}

void workerThread( zmq::context_t &ctx, uint32_t id ) {
	RandomNumberGenerator workLoad{ 1, 1000 };
	RandomNumberGenerator numReplies{ 1, 5 };
	zmq::socket_t worker{ ctx, zmq::socket_type::dealer };

	zmq::active_poller_t poller;
	poller.add( worker, zmq::event_flags::pollin, [ &worker, &workLoad, &numReplies, &id ]( zmq::event_flags flag ) {
		auto identity = ZmqUtil::recvString( worker, zmq::recv_flags::dontwait );
		if ( !identity.has_value() ) {
			spdlog::error( "Error in receiving identity" );
			return;
		}
		auto message = ZmqUtil::recvString( worker, zmq::recv_flags::dontwait );
		if ( !message.has_value() ) {
			spdlog::error( "Error in receiving message" );
			return;
		}
		auto replies = numReplies.generate();
		// show that we can send multiple replies
		for ( int32_t i = 0; i < replies; i++ ) {
			int32_t sleepTime = workLoad.generate();
			spdlog::info( "worker {} sleeps for {} milliseconds", id, sleepTime );
			std::this_thread::sleep_for( std::chrono::milliseconds( sleepTime ) );
			ZmqUtil::sendString( worker, identity.value(), zmq::send_flags::sndmore );
			ZmqUtil::sendString( worker, message.value() );
		}
	} );

	worker.connect( "inproc://back" );
	while ( mIsRunning ) {
		try {
			[[maybe_unused]] auto _ = poller.wait( std::chrono::milliseconds( 100 ) );
		} catch ( zmq::error_t &e ) {
			spdlog::info( "worker {} exits!", id );
		}
	}
}

void usageError( std::string_view prog, std::string_view msg ) {
	std::cerr << msg << "\n";
	std::cout << std::format( "Usage: {} clients workers", prog );
	exit( EXIT_FAILURE );
}

int main( int argc, char *argv[] ) {
	if ( argc != 3 ) {
		usageError( argv[ 0 ], "Number of arguments must be 3" );
	}

	// Catch SIGINT signal
	struct sigaction callback;
	// interrupt zmq read
	callback.sa_flags = 0;
	callback.sa_handler = handler;
	sigaction( SIGINT, &callback, nullptr );

	int32_t numClients{ 0 };
	int32_t numWorkers{ 0 };
	try {
		numClients = std::stoi( argv[ 1 ] );
		numWorkers = std::stoi( argv[ 2 ] );
	} catch ( const std::invalid_argument &e ) {
		usageError( argv[ 0 ], "Number of clients and workers must be an integer" );
	}

	zmq::context_t ctx;
	zmq::socket_t front{ ctx, zmq::socket_type::router };
	front.bind( "tcp://*:5570" );

	zmq::socket_t back{ ctx, zmq::socket_type::dealer };
	back.bind( "inproc://back" );

	std::vector<std::thread> clients;
	for ( int32_t i = 0; i < numClients; i++ ) {
		clients.emplace_back( clientThread, std::ref( ctx ), i );
	}

	std::vector<std::thread> workers;
	for ( int32_t i = 0; i < numWorkers; i++ ) {
		workers.emplace_back( workerThread, std::ref( ctx ), i );
	}

	try {
		zmq::proxy( front, back );
	} catch ( zmq::error_t &e ) {
		spdlog::info( "Proxy exits!" );
	}

	for ( int32_t i = 0; i < numClients; i++ ) {
		clients[ i ].join();
	}

	for ( int32_t i = 0; i < numWorkers; i++ ) {
		workers[ i ].join();
	}

	return 0;
}