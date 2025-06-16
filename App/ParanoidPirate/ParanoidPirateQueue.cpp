#include "WorkerQueue.h"

#include <chrono>
#include <iterator>
#include <spdlog/spdlog.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <ZmqUtil.h>

namespace {
	void handleFront( zmq::socket_t &front, zmq::socket_t &back, WorkerQueue &workerQueue ) {
		if ( workerQueue.isEmpty() ) {
			spdlog::warn( "No idle worker!" );
			return;
		}
		auto idleWorker = workerQueue.pop();
		auto rawFrames = ZmqUtil::recvAllStrings( front );
		if ( !rawFrames.has_value() ) {
			spdlog::error( "Failed to receive from frontend!" );
			return;
		}
		std::vector<std::string> frames{ idleWorker };
		std::copy( rawFrames.value().begin(), rawFrames.value().end(), std::back_inserter( frames ) );
		ZmqUtil::sendAllStrings( back, frames );
	}

	void handleBack( zmq::socket_t &front, zmq::socket_t &back, WorkerQueue &workerQueue ) {
		auto rawFrames = ZmqUtil::recvAllStrings( back );
		if ( !rawFrames.has_value() ) {
			spdlog::error( "Failed to receive messages from backend!" );
			return;
		}
		auto frames = rawFrames.value();
		auto workerIdentity = frames.front();
		if ( frames.size() == 2 ) {
			if ( frames.back() == "ready" ) {
				spdlog::debug( "Received ready from {}", workerIdentity );
				workerQueue.remove( workerIdentity );
				workerQueue.add( workerIdentity );
			} else if ( frames.back() == "heartbeat" ) {
				spdlog::debug( "Received heartbeat from {}", workerIdentity );
				workerQueue.refresh( workerIdentity );
			} else {
				spdlog::error( "Invalid message from {}", workerIdentity );
			}
		} else {
			std::vector<std::string> response{ frames.begin() + 1, frames.end() };
			ZmqUtil::sendAllStrings( front, response );
			workerQueue.add( workerIdentity );
		}
	}
}

int main( int argc, char *argv[] ) {
	spdlog::set_level( spdlog::level::debug );
	zmq::context_t ctx;
	zmq::socket_t front{ ctx, zmq::socket_type::router };
	front.bind( "tcp://*:5555" );

	zmq::socket_t back{ ctx, zmq::socket_type::router };
	back.bind( "tcp://*:5556" );

	WorkerQueue workerQueue;
	zmq::active_poller_t poller;
	poller.add( front, zmq::event_flags::pollin, [ &front, &back, &workerQueue ]( zmq::event_flags flag ) {
		handleFront( front, back, workerQueue );
	} );
	poller.add( back, zmq::event_flags::pollin, [ &front, &back, &workerQueue ]( zmq::event_flags flag ) {
		handleBack( front, back, workerQueue );
	} );

	while ( 1 ) {
		[[maybe_unused]] auto _ = poller.wait( std::chrono::milliseconds( 100 ) );
		workerQueue.sendHeartbeat( back );
		workerQueue.purge();
	}

	return 0;
}