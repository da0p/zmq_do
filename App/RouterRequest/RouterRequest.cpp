#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

#include <spdlog/spdlog.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <RandomNumberGenerator.h>
#include <StopTimer.h>
#include <ZmqUtil.h>

void doWork( zmq::context_t &ctx ) {
	zmq::socket_t worker{ ctx, zmq::socket_type::req };
	worker.connect( "ipc://router.ipc" );

	RandomNumberGenerator rand{ 1, 500 };
	size_t total = 0;
	while ( 1 ) {
		ZmqUtil::sendString( worker, "ready" );
		auto msg = ZmqUtil::recvString( worker );
		if ( msg.value() != "stop" ) {
			total++;
		} else {
			spdlog::info( "Processed work: {} tasks", total );
			break;
		}
		std::this_thread::sleep_for( std::chrono::milliseconds( rand.generate() ) );
	}
}

int main( int argc, char *argv[] ) {
	uint32_t numWorkers{ 0 };
	if ( argc != 2 ) {
		std::cerr << "Number of arguments must be 2\n";
		exit( EXIT_FAILURE );
	}

	numWorkers = std::stoi( argv[ 1 ] );
	spdlog::info( "We have {} workers", numWorkers );

	zmq::context_t ctx;
	zmq::socket_t broker{ ctx, zmq::socket_type::router };
	broker.bind( "ipc://router.ipc" );

	std::vector<std::thread> workers;
	for ( size_t i = 0; i < numWorkers; i++ ) {
		workers.push_back( std::thread( doWork, std::ref( ctx ) ) );
	}

	size_t stoppedWorker{ 0 };
	StopTimer stopTimer;
	stopTimer.start();
	while ( 1 ) {
		// receive identity part
		auto identity = ZmqUtil::recvString( broker );
		// receive envelope delimiter
		[[maybe_unused]] auto _ = ZmqUtil::recvString( broker );
		_ = ZmqUtil::recvString( broker );

		// We first put the identity so that ROUTER knows where to send
		ZmqUtil::sendString( broker, identity.value(), zmq::send_flags::sndmore );
		// send an empty delimiter
		ZmqUtil::sendString( broker, "", zmq::send_flags::sndmore );

		if ( stopTimer.getTimeElasped() < std::chrono::seconds( 5 ) ) {
			ZmqUtil::sendString( broker, "work" );
		} else {
			ZmqUtil::sendString( broker, "stop" );
			stoppedWorker++;
			if ( stoppedWorker == numWorkers ) {
				break;
			}
		}
	}

	for ( size_t i = 0; i < numWorkers; i++ ) {
		workers[ i ].join();
	}

	return 0;
}