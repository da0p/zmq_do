#include "MajordomoClient.h"
#include "MajordomoClientMessage.h"
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
	auto discovery = MajordomoClientMessage::DiscoveryReply::from( rawFrames.value() );
	if ( discovery.has_value() ) {
		spdlog::info(
		  "Received discovery reply for service={}, status={}", discovery.value().serviceName, discovery.value().status );
	} else {
		mReply = MajordomoClientMessage::Reply::from( rawFrames.value() );
		if ( !mReply.has_value() ) {
			spdlog::error( "Failed to parse reply" );
			ZmqUtil::dump( rawFrames.value() );
			return;
		}
		spdlog::info( "Received reply: version: {}, serviceName: {}", mReply.value().version, mReply.value().serviceName );
	}
	// we finished! yayay
	mIsRunning = false;
}

void MajordomoClient::send( const std::string &service, const std::vector<uint8_t> &messageBody ) {
	MajordomoClientMessage::Request request{ gMajVer, service, messageBody };
	ZmqUtil::sendAllFrames( *mSocket, MajordomoClientMessage::Request::to( request ) );
	mIsRunning = true;
}

void MajordomoClient::ping( const std::string &service ) {
	MajordomoClientMessage::DiscoveryRequest request{ .header = gDiscoveryService, .serviceName = service };
	ZmqUtil::sendAllFrames( *mSocket, MajordomoClientMessage::DiscoveryRequest::to( request ) );
}

std::optional<MajordomoClientMessage::Reply> MajordomoClient::recv( std::chrono::milliseconds timeout ) {
	auto evt = mPoller.wait( timeout );
	if ( !evt ) {
		spdlog::warn( "No reply, reconnecting..." );
		connect();
		return {};
	} else {
		return mReply;
	}
}