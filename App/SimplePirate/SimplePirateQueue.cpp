#include <queue>

#include <spdlog/spdlog.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <ZmqUtil.h>

namespace {
	void handleFront( zmq::socket_t &front, zmq::socket_t &back, std::queue<std::string> &idleWorkers ) {
		if ( idleWorkers.empty() ) {
			spdlog::warn( "No idle worker" );
			return;
		}
		auto workerAddr = idleWorkers.front();
		idleWorkers.pop();

		auto rawFrames = ZmqUtil::recvAllStrings( front );
		if ( !rawFrames.has_value() ) {
			spdlog::error( "Failed to receive frames from fronend" );
			return;
		}
		std::vector<std::string> frames{ workerAddr, "" };
		std::copy( rawFrames.value().begin(), rawFrames.value().end(), std::back_inserter( frames ) );
		ZmqUtil::sendAllStrings( back, frames );
	}

	void handleBack( zmq::socket_t &front, zmq::socket_t &back, std::queue<std::string> &idleWorkers ) {
		auto rawFrames = ZmqUtil::recvAllStrings( back );
		if ( !rawFrames.has_value() ) {
			spdlog::error( "Failed to receive frames from backend" );
			return;
		}

		auto frames = rawFrames.value();
		idleWorkers.push( frames.front() );

		if ( frames.back() != "ready" ) {
			std::vector<std::string> reply{ frames.begin() + 2, frames.end() };
			ZmqUtil::sendAllStrings( front, reply );
		}
	}
}

int main( int argc, char *argv[] ) {
	zmq::context_t ctx;
	zmq::socket_t front{ ctx, zmq::socket_type::router };
	front.bind( "tcp://*:5555" );

	zmq::socket_t back{ ctx, zmq::socket_type::router };
	back.bind( "tcp://*:5556" );

	std::queue<std::string> idleWorkers;

	zmq::active_poller_t poller;
	poller.add( front, zmq::event_flags::pollin, [ &front, &back, &idleWorkers ]( zmq::event_flags flag ) {
		handleFront( front, back, idleWorkers );
	} );

	poller.add( back, zmq::event_flags::pollin, [ &front, &back, &idleWorkers ]( zmq::event_flags flag ) {
		handleBack( front, back, idleWorkers );
	} );

	while ( 1 ) {
		if ( idleWorkers.empty() ) {
			poller.modify( front, zmq::event_flags::none );
		} else {
			poller.modify( front, zmq::event_flags::pollin );
		}
		[[maybe_unused]] auto _ = poller.wait( std::chrono::seconds( 100 ) );
	}

	return 0;
}