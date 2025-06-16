#include <spdlog/spdlog.h>
#include <thread>
#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <RandomNumberGenerator.h>
#include <ZmqUtil.h>

int main( int argc, char *argv[] ) {
	zmq::context_t ctx;
	zmq::socket_t worker{ ctx, zmq::socket_type::req };
	worker.connect( "tcp://localhost:5556" );

	RandomNumberGenerator idGen{ 1, 1000000 };
	auto identity = std::format( "worker-{}", idGen.generate() );

	spdlog::info( "Identity: {} ready!", identity );
	ZmqUtil::sendString( worker, "ready" );

	RandomNumberGenerator rndGen{ 0, 5 };
	int32_t cycles{ 0 };
	while ( 1 ) {
		auto rawFrames = ZmqUtil::recvAllStrings( worker );
		if ( !rawFrames.has_value() ) {
			spdlog::error( "Failed to receive frames!" );
			break;
		}
		cycles++;
		if ( cycles > 3 && rndGen.generate() == 0 ) {
			spdlog::error( "Worker {} crashed!", identity );
			break;
		} else {
			if ( cycles > 3 && rndGen.generate() == 0 ) {
				spdlog::error( "Worker {} CPU overload!", identity );
				std::this_thread::sleep_for( std::chrono::seconds( 5 ) );
			}
			auto frames = rawFrames.value();
			spdlog::info( "Worker {}: normal reply - {}", identity, frames.back() );
			std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
			ZmqUtil::sendAllStrings( worker, frames );
		}
	}

	return 0;
}