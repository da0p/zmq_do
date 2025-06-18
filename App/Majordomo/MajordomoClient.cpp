#include "MajordomoClient.h"
#include "ZmqUtil.h"

#include <chrono>
#include <spdlog/spdlog.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <Common.h>
#include <RandomString.h>

MajordomoClient::MajordomoClient( const std::string &brokerAddr ) : mBrokerAddr{ brokerAddr } {
	connect();
}

void MajordomoClient::stop() {
	spdlog::warn( "Interrupt received. Killing client!" );
	mIsRunning = false;
}

void MajordomoClient::connect() {
	if ( mSocket ) {
		mPoller.remove( *mSocket );
		mSocket.reset();
	}
	mSocket = std::make_unique<zmq::socket_t>( mZmqCtx, zmq::socket_type::req );
	auto identity = RandomString::generate( 5 );
	mSocket->set( zmq::sockopt::routing_id, identity );
	mSocket->set( zmq::sockopt::linger, 0 );
	mSocket->connect( mBrokerAddr );
	spdlog::info( "Connecting to broker at {} with identity {}", mBrokerAddr, identity );
	mPoller.add( *mSocket, zmq::event_flags::pollin, [ this ]( zmq::event_flags flag ) { handleResponse(); } );
}

void MajordomoClient::handleResponse() {
	auto rawFrames = ZmqUtil::recvAllFrames( *mSocket );
	if ( !rawFrames.has_value() ) {
		spdlog::error( "Failed to receive frames!" );
		return;
	}
	mReply = toReply( rawFrames.value() );
	if ( !mReply.has_value() ) {
		spdlog::error( "Failed to parse reply" );
		return;
	}
	spdlog::info( "Received reply: version: {}, clientAddr: {}", mReply.value().version, mReply.value().clientAddr );
	ZmqUtil::dump( mReply.value().body );
	// we finished! yayay
	mIsRunning = false;
}

std::optional<Reply> MajordomoClient::send( const std::string &service,
                                            const std::vector<uint8_t> &messageBody,
                                            std::chrono::milliseconds timeout,
                                            uint32_t retries ) {
	Request request{ .version = gMajVer, .clientAddr = service, .body = messageBody };
	uint32_t currentRetries = 0;
	mIsRunning = true;
	while ( currentRetries < retries && mIsRunning ) {
		ZmqUtil::sendAllFrames( *mSocket, toFrames( request ) );
		auto evt = mPoller.wait( timeout );
		if ( !evt ) {
			currentRetries++;
			spdlog::warn( "No reply, reconnecting..." );
			connect();
		}
	}
	if ( currentRetries >= retries ) {
		spdlog::warn( "Too many failed attempts. Abort!" );
	}

	return mReply;
}