#include <BinaryStarClient.h>

#include <spdlog/spdlog.h>
#include <zmq.hpp>

namespace {
	constexpr auto gPollingTimeout = std::chrono::seconds( 1 );
	constexpr auto gSettleDelay = std::chrono::seconds( 2 );
}

BinaryStarClient::BinaryStarClient( const std::string &primaryAddr, const std::string &failoverAddr ) :
        mAddresses{ primaryAddr, failoverAddr } {
}

void BinaryStarClient::connect() {
	if ( mSocket ) {
		mPoller.remove( *mSocket );
		mSocket.reset();
	}

	mSocket = std::make_unique<zmq::socket_t>( mZmqCtx, zmq::socket_type::req );
	mSocket->set( zmq::sockopt::linger, 0 );

	auto addr = mAddresses[ mAddrTurn % 2 ];
	spdlog::info( "Connecting to {} ...", addr );
	setupCallback();
	mSocket->connect( addr );
	mAddrTurn++;
}

void BinaryStarClient::setupCallback() {
	mPoller.add( *mSocket, zmq::event_flags::pollin, [ this ]( zmq::event_flags flag ) {
		zmq::message_t reply;
		auto res = mSocket->recv( reply, zmq::recv_flags::none );
		if ( !res.has_value() ) {
			spdlog::error( "Failed to receive reply!" );
			return;
		}

		if ( reply.to_string().compare( std::to_string( mSeq ) ) == 0 ) {
			spdlog::info( "Server replied Ok ({})", reply.to_string() );
			mExpectReply = false;
			std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
		} else {
			spdlog::error( "Bad reply from server: {}", reply.to_string() );
		}
	} );
}

void BinaryStarClient::run() {
	connect();
	while ( 1 ) {
		zmq::message_t req{ std::to_string( ++mSeq ) };
		mSocket->send( req, zmq::send_flags::none );
		mExpectReply = true;
		while ( mExpectReply ) {
			auto recv = mPoller.wait( gPollingTimeout );
			if ( !recv ) {
				spdlog::warn( "No response from server, failing over" );
				std::this_thread::sleep_for( gSettleDelay );
				connect();
				zmq::message_t req{ std::to_string( mSeq ) };
				mSocket->send( req, zmq::send_flags::none );
			}
		}
	}
}