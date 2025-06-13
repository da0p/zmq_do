#include <spdlog/spdlog.h>
#include <thread>
#include <zmq.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <ZmqUtil.h>

namespace {
	constexpr auto gRequestRetries = 3;
	constexpr auto gRequestTimeout = std::chrono::milliseconds( 2500 );

	zmq::socket_t createSocket( zmq::context_t &ctx ) {
		spdlog::info( "Connecting to server...\n" );
		zmq::socket_t socket{ ctx, zmq::socket_type::req };
		socket.connect( "tcp://localhost:5555" );
		socket.set( zmq::sockopt::linger, 0 );
		return socket;
	}

	void handleResponse( zmq::socket_t &socket, uint32_t seq, bool &expectReply ) {
		auto response = ZmqUtil::recvString( socket );
		if ( !response.has_value() ) {
			return;
		}
		auto receivedSeq = static_cast<uint32_t>( std::stoi( response.value() ) );
		if ( receivedSeq == seq ) {
			expectReply = false;
			spdlog::info( "server replied ok ({})", receivedSeq );
		} else {
			spdlog::warn( "wrong received seq ({})", receivedSeq );
		}
	}
}

int main( int argc, char *argv[] ) {

	zmq::context_t ctx;
	auto socket = createSocket( ctx );

	uint32_t seq{ 0 };
	uint32_t retries{ 0 };
	bool expectReply{ true };
	zmq::active_poller_t poller;
	poller.add( socket, zmq::event_flags::pollin, [ &socket, &seq, &expectReply ]( zmq::event_flags flag ) {
		handleResponse( socket, seq, expectReply );
	} );

	auto request = std::to_string( ++seq );
	ZmqUtil::sendString( socket, request );
	while ( expectReply ) {
		auto res = poller.wait( gRequestTimeout );
		if ( !res ) {
			if ( retries < gRequestRetries ) {
				spdlog::warn( "no reponse from server. Retrying..." );
				poller.remove( socket );
				socket.close();
				socket = createSocket( ctx );
				poller.add( socket, zmq::event_flags::pollin, [ &socket, &seq, &expectReply ]( zmq::event_flags flag ) {
					handleResponse( socket, seq, expectReply );
				} );
				ZmqUtil::sendString( socket, request );
				retries++;
			} else {
				spdlog::error( "server offline. Abandoning..." );
				expectReply = false;
			}
		}
	}

	return 0;
}