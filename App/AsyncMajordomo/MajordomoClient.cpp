#include "MajordomoClient.h"
#include "MajordomoClientData.h"
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
	mSocket = std::make_unique<zmq::socket_t>( mZmqCtx, zmq::socket_type::dealer );
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
	mReply = MajordomoClientCmd::Reply::from( rawFrames.value() );
	if ( !mReply.has_value() ) {
		spdlog::error( "Failed to parse reply" );
		ZmqUtil::dump( rawFrames.value() );
		return;
	}
	spdlog::info( "Received reply: version: {}, serviceName: {}", mReply.value().version, mReply.value().serviceName );
	// we finished! yayay
	mIsRunning = false;
}

void MajordomoClient::send( const std::string &service, const std::vector<uint8_t> &messageBody ) {
	MajordomoClientCmd::Request request{ gMajVer, service, messageBody };
	ZmqUtil::sendAllFrames( *mSocket, MajordomoClientCmd::Request::to( request ) );
	mIsRunning = true;
}

std::optional<MajordomoClientCmd::Reply> MajordomoClient::recv( std::chrono::milliseconds timeout ) {
	auto evt = mPoller.wait( timeout );
	if ( !evt ) {
		spdlog::warn( "No reply, reconnecting..." );
		connect();
		return {};
	} else {
		return mReply;
	}
}