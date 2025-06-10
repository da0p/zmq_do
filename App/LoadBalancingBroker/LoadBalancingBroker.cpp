#include <csignal>
#include <cstdlib>
#include <iostream>
#include <queue>
#include <stdexcept>
#include <system_error>
#include <thread>

#include <spdlog/spdlog.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <ZmqUtil.h>

static volatile bool mIsRunning{ true };

static void handler( int sig ) {
	if ( sig == SIGINT ) {
		mIsRunning = false;
	}
}

void clientThread( zmq::context_t &ctx, uint32_t id ) {
	spdlog::info( "Running from client thread {}", id );
	zmq::socket_t client{ ctx, zmq::socket_type::req };
	client.set( zmq::sockopt::routing_id, std::format( "client-{}", id ) );
	client.connect( "ipc://front.ipc" );
	ZmqUtil::sendString( client, "Hello" );
	auto reply = ZmqUtil::recvString( client );
	spdlog::info( "Client: {}", reply.value() );
}

void workerThread( zmq::context_t &ctx, uint32_t id ) {
	zmq::socket_t worker{ ctx, zmq::socket_type::req };
	worker.set( zmq::sockopt::routing_id, std::format( "worker-{}", id ) );
	worker.connect( "ipc://back.ipc" );

	// notify ready
	ZmqUtil::sendString( worker, "ready" );

	while ( mIsRunning ) {
		try {
			auto address = ZmqUtil::recvString( worker, zmq::recv_flags::dontwait );
			if ( !address.has_value() ) {
				continue;
			}

			auto emptyDelimiter = ZmqUtil::recvString( worker, zmq::recv_flags::dontwait );
			if ( !emptyDelimiter.has_value() ) {
				continue;
			}

			auto request = ZmqUtil::recvString( worker, zmq::recv_flags::dontwait );
			if ( !request.has_value() ) {
				continue;
			}

			ZmqUtil::sendString( worker, address.value(), zmq::send_flags::sndmore );
			ZmqUtil::sendString( worker, "", zmq::send_flags::sndmore );
			ZmqUtil::sendString( worker, "ok" );
		} catch ( zmq::error_t &e ) {
			std::cerr << "Interrupt received. Exit!" << std::endl;
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

	int32_t numClients{ 0 };
	int32_t numWorkers{ 0 };
	try {
		numClients = std::stoi( argv[ 1 ] );
		numWorkers = std::stoi( argv[ 2 ] );
	} catch ( const std::invalid_argument &e ) {
		usageError( argv[ 0 ], "Number of clients and workers must be an integer" );
	}

	// set up interrupt
	struct sigaction callback;
	// set sa_flags = 0 to interrupt zmq read
	callback.sa_flags = 0;
	callback.sa_handler = handler;
	sigaction( SIGINT, &callback, nullptr );

	zmq::context_t ctx{ 1 };
	zmq::socket_t front{ ctx, zmq::socket_type::router };
	front.bind( "ipc://front.ipc" );

	zmq::socket_t back{ ctx, zmq::socket_type::router };
	back.bind( "ipc://back.ipc" );

	std::vector<std::thread> clients;
	for ( auto i = 0; i < numClients; i++ ) {
		clients.emplace_back( clientThread, std::ref( ctx ), i );
	}

	std::vector<std::thread> workers;
	for ( auto i = 0; i < numWorkers; i++ ) {
		workers.emplace_back( workerThread, std::ref( ctx ), i );
	}

	std::queue<std::string> idleWorkers;
	zmq::active_poller_t poller;
	poller.add( back, zmq::event_flags::pollin, [ &front, &back, &idleWorkers ]( zmq::event_flags flag ) {
		// we got this event, that means a worker just said that it's ready to
		// take a new job
		auto workerAddress = ZmqUtil::recvString( back );
		if ( !workerAddress.has_value() ) {
			spdlog::error( "Error in receiving worker address" );
			return;
		}

		[[maybe_unused]] auto emptyDelimiter = ZmqUtil::recvString( back );
		idleWorkers.push( workerAddress.value() );

		// the 3rd frame can be 'ready' or a client address
		auto clientAddr = ZmqUtil::recvString( back );
		if ( !clientAddr.has_value() ) {
			spdlog::error( "Error in receiving client address/notify signal" );
			return;
		}
		if ( clientAddr.value() != "ready" ) {
			[[maybe_unused]] auto emptyDelimiter = ZmqUtil::recvString( back );
			auto message = ZmqUtil::recvString( back );
			if ( !message.has_value() ) {
				spdlog::error( "Error in receiving reply message from workers" );
				return;
			}

			ZmqUtil::sendString( front, clientAddr.value(), zmq::send_flags::sndmore );
			ZmqUtil::sendString( front, "", zmq::send_flags::sndmore );
			ZmqUtil::sendString( front, message.value() );
		}
	} );

	poller.add( front, zmq::event_flags::pollin, [ &front, &back, &idleWorkers ]( zmq::event_flags flag ) {
		if ( idleWorkers.empty() ) {
			spdlog::info( "No idle workers" );
			return;
		}
		auto clientAddress = ZmqUtil::recvString( front );
		if ( !clientAddress.has_value() ) {
			spdlog::error( "Error in receiving client address" );
			return;
		}

		[[maybe_unused]] auto emptyDelimiter = ZmqUtil::recvString( front );

		auto request = ZmqUtil::recvString( front );
		if ( !request.has_value() ) {
			spdlog::error( "Error in receiving requests" );
			return;
		}

		auto workerAddress = idleWorkers.front();
		idleWorkers.pop();
		ZmqUtil::sendString( back, workerAddress, zmq::send_flags::sndmore );
		ZmqUtil::sendString( back, "", zmq::send_flags::sndmore );
		ZmqUtil::sendString( back, clientAddress.value(), zmq::send_flags::sndmore );
		ZmqUtil::sendString( back, "", zmq::send_flags::sndmore );
		ZmqUtil::sendString( back, request.value() );
	} );

	while ( mIsRunning ) {
		try {
			[[maybe_unused]] auto _ = poller.wait( std::chrono::milliseconds( 100 ) );
		} catch ( zmq::error_t &e ) {
			std::cerr << "Interrupt received, exit!" << std::endl;
		}
	}

	spdlog::info( "Joining client threads!" );
	for ( auto i = 0; i < numClients; i++ ) {
		clients[ i ].join();
	}

	spdlog::info( "Joining worker threads!" );
	for ( auto i = 0; i < numWorkers; i++ ) {
		workers[ i ].join();
	}

	return 0;
}