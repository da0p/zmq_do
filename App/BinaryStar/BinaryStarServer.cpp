#include "BinaryStarServer.h"

#include <chrono>

#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>

namespace {
	constexpr auto gSelfStatePubInterval = std::chrono::seconds( 2 );
}

BinaryStarServer::BinaryStarServer( bool primary,
                                    const std::string &handleReqSocket,
                                    const std::string &selfStatusPubSocket,
                                    const std::string &peerStatusSubSocket ) :
        mBinaryStarFsm{ primary ? StateName::Primary : StateName::Backup } {
	setup( handleReqSocket, selfStatusPubSocket, peerStatusSubSocket );
}

void BinaryStarServer::setup( const std::string &handleReqSocket,
                              const std::string &selfStatusPubSocket,
                              const std::string &peerStatusSubSocket ) {
	mHandleReqSocket = std::make_unique<zmq::socket_t>( mZmqCtx, zmq::socket_type::router );
	mHandleReqSocket->bind( handleReqSocket );

	mSelfStatusPubSocket = std::make_unique<zmq::socket_t>( mZmqCtx, zmq::socket_type::pub );
	mSelfStatusPubSocket->bind( selfStatusPubSocket );

	mPeerStatusSubSocket = std::make_unique<zmq::socket_t>( mZmqCtx, zmq::socket_type::sub );
	mPeerStatusSubSocket->set( zmq::sockopt::subscribe, "" );
	mPeerStatusSubSocket->connect( peerStatusSubSocket );

	mBinaryStarFsm.set( StateName::Primary, EventName::ClientRequest, [ this ] { respond(); } );
	mBinaryStarFsm.set( StateName::Active, EventName::ClientRequest, [ this ] { respond(); } );
	mBinaryStarFsm.set( StateName::Passive, EventName::ClientRequest, [ this ] { respond(); } );
}

void BinaryStarServer::run() {
	mPoller.add(
	  *mHandleReqSocket, zmq::event_flags::pollin, [ this ]( zmq::event_flags flag ) { handleRequestReceived(); } );

	mPoller.add( *mPeerStatusSubSocket, zmq::event_flags::pollin, [ this ]( zmq::event_flags flag ) {
		handlePeerStateReceived();
	} );

	mStatePubDeadline = std::chrono::steady_clock::now();
	while ( 1 ) {
		[[maybe_unused]] auto _ = mPoller.wait( std::chrono::milliseconds( 100 ) );
		pubSelfState();
	}
}

void BinaryStarServer::handleRequestReceived() {
	zmq::multipart_t req;
	auto res = req.recv( *mHandleReqSocket );
	if ( !res ) {
		spdlog::error( "Failed to receive request!" );
		return;
	}

	auto clientId = req.popstr();
	[[maybe_unused]] auto delimiter = req.pop();
	mResponse.clientId = clientId;
	mResponse.body = req.popstr();
	spdlog::info( "Received request body={}", mResponse.body );
	mBinaryStarFsm.trigger( EventName::ClientRequest );
}

void BinaryStarServer::respond() {
	zmq::multipart_t response;
	response.push_back( zmq::message_t{ mResponse.clientId } );
	response.push_back( zmq::message_t{} );
	response.push_back( zmq::message_t{ mResponse.body } );
	response.send( *mHandleReqSocket );
}

void BinaryStarServer::handlePeerStateReceived() {
	zmq::multipart_t peerState;
	auto res = peerState.recv( *mPeerStatusSubSocket );
	if ( !res ) {
		spdlog::error( "Failed to receive peer state!" );
		return;
	}

	auto state = peerState.poptyp<uint8_t>();
	spdlog::info( "Received peer state = {}", magic_enum::enum_name( static_cast<StateName>( state ) ) );
	mBinaryStarFsm.trigger( toPeerEvt( static_cast<StateName>( state ) ) );
}

EventName BinaryStarServer::toPeerEvt( StateName state ) {
	switch ( state ) {
		case StateName::Active:
			return EventName::PeerActive;
		case StateName::Passive:
			return EventName::PeerPassive;
		case StateName::Backup:
			return EventName::PeerBackup;
		case StateName::Primary:
			return EventName::PeerPrimary;
		case StateName::Unknown:
		default:
			return EventName::Unknown;
	}
}

void BinaryStarServer::pubSelfState() {
	if ( std::chrono::steady_clock::now() > mStatePubDeadline ) {
		zmq::multipart_t state;
		state.pushtyp( static_cast<uint8_t>( mBinaryStarFsm.getState() ) );
		state.send( *mSelfStatusPubSocket );
		mStatePubDeadline = std::chrono::steady_clock::now() + gSelfStatePubInterval;
	}
}