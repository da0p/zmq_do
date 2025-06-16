#include <chrono>

#include <spdlog/spdlog.h>
#include <thread>
#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <RandomNumberGenerator.h>
#include <ZmqUtil.h>

namespace {
	constexpr auto gWorkerHeartbeatExpiry = std::chrono::milliseconds( 1000 );
	constexpr auto gQueueHeartbeatExpiry = std::chrono::milliseconds( 3500 );
	zmq::socket_t createSocket( zmq::context_t &ctx, const std::string &identity ) {
		zmq::socket_t worker{ ctx, zmq::socket_type::dealer };
		worker.set( zmq::sockopt::routing_id, identity );
		worker.set( zmq::sockopt::linger, 0 );
		worker.connect( "tcp://localhost:5556" );
		spdlog::info( "Worker {} ready!", identity );
		ZmqUtil::sendString( worker, "ready" );
		return worker;
	}

	void sendHeartbeat( zmq::socket_t &socket, std::chrono::steady_clock::time_point &deadline, const std::string &identity ) {
		if ( std::chrono::steady_clock::now() > deadline ) {
			deadline = std::chrono::steady_clock::now() + gWorkerHeartbeatExpiry;
			spdlog::info( "Worker {} heartbeat", identity );
			ZmqUtil::sendString( socket, "heartbeat" );
		}
	}

	void handleFront( zmq::socket_t &socket,
	                  uint32_t &cycles,
	                  std::chrono::steady_clock::time_point &queueHeartbeatDeadline,
	                  const std::string &identity,
	                  bool &isRunning ) {
		auto rawFrames = ZmqUtil::recvAllStrings( socket );
		if ( !rawFrames.has_value() ) {
			spdlog::error( "Failed to receive message!" );
			return;
		}
		auto frames = rawFrames.value();
		if ( frames.size() == 1 ) {
			// heartbeat frame
			if ( frames.front() == "heartbeat" ) {
				spdlog::info( "Received heartbeat from queue" );
				auto newQueueDeadline = std::chrono::steady_clock::now() + gQueueHeartbeatExpiry;
				spdlog::info( "queueHeartbeatDeadline changed: {} -> {}",
				              queueHeartbeatDeadline.time_since_epoch().count(),
				              newQueueDeadline.time_since_epoch().count() );
				queueHeartbeatDeadline = newQueueDeadline;
			} else {
				spdlog::error( "Invalid heartbeat message!" );
			}
		} else if ( frames.size() == 3 ) {
			RandomNumberGenerator rndGen{ 0, 5 };
			cycles++;
			if ( cycles > 3 && rndGen.generate() == 0 ) {
				spdlog::info( "Worker {} crashed!", identity );
				isRunning = false;
			} else if ( cycles > 3 && rndGen.generate() == 0 ) {
				spdlog::info( "Worker {}: CPU overload!", identity );
				std::this_thread::sleep_for( std::chrono::seconds( 5 ) );
			}
			spdlog::info( "Normal reply {}", frames.back() );
			std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
			ZmqUtil::sendAllStrings( socket, frames );
		}
	}
}

int main( int argc, char *argv[] ) {
	RandomNumberGenerator rnd{ 1, 1000000 };
	auto identity = std::format( "worker-{}", rnd.generate() );
	zmq::context_t ctx;
	auto worker = createSocket( ctx, identity );
	uint32_t cycles = 0;
	auto workerHeartbeatDeadline = std::chrono::steady_clock::now() + gWorkerHeartbeatExpiry;
	auto queueHeartbeatDeadline = std::chrono::steady_clock::now() + gQueueHeartbeatExpiry;
	bool isRunning{ true };
	zmq::active_poller_t poller;
	poller.add( worker,
	            zmq::event_flags::pollin,
	            [ &worker, &cycles, &queueHeartbeatDeadline, &identity, &isRunning ]( zmq::event_flags flag ) {
		            handleFront( worker, cycles, queueHeartbeatDeadline, identity, isRunning );
	            } );

	while ( isRunning ) {
		[[maybe_unused]] auto _ = poller.wait( std::chrono::milliseconds( 100 ) );
		if ( std::chrono::steady_clock::now() > queueHeartbeatDeadline ) {
			spdlog::info( "Lost heartbeat. now = {}, queueHeartbeatDeadline = {}",
			              std::chrono::steady_clock::now().time_since_epoch().count(),
			              queueHeartbeatDeadline.time_since_epoch().count() );
			auto interval = std::chrono::seconds( 3 );
			spdlog::info( "Reconnect in {} seconds", interval.count() );
			queueHeartbeatDeadline += interval * 2;
			std::this_thread::sleep_for( interval );
			poller.remove( worker );
			worker.close();
			worker = createSocket( ctx, identity );
			poller.add( worker,
			            zmq::event_flags::pollin,
			            [ &worker, &cycles, &queueHeartbeatDeadline, &identity, &isRunning ]( zmq::event_flags flag ) {
				            handleFront( worker, cycles, queueHeartbeatDeadline, identity, isRunning );
			            } );
		}
		sendHeartbeat( worker, workerHeartbeatDeadline, identity );
	}

	return 0;
}