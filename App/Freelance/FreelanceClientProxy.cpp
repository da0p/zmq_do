#include <FreelanceClientProxy.h>

#include <spdlog/spdlog.h>

#include <ZmqUtil.h>

namespace {
	constexpr auto gServerHeartbeatExpiry = std::chrono::seconds( 3 );
}

FreelanceProxy::FreelanceProxy( zmq::context_t &ctx,
                                const std::vector<uint16_t> &ports,
                                const std::string &hbEndpoint,
                                const std::string &reqEndpoint ) :
        mFrontSocket{ ctx, zmq::socket_type::router },
        mHeartbeatSocket{ ctx, zmq::socket_type::pair },
        mReqSocket{ ctx, zmq::socket_type::pair } {
	setupServers( ports );
	setupFrontSocket();
	setupHeartbeatSocket( hbEndpoint );
	setupReqSocket( reqEndpoint );
}

void FreelanceProxy::setupReqSocket( const std::string &reqEndpoint ) {
	mReqSocket.bind( reqEndpoint );
	mPoller.add( mReqSocket, zmq::event_flags::pollin, [ this ]( zmq::event_flags flag ) { forwardReq(); } );
}

void FreelanceProxy::forwardReq() {
	zmq::message_t req;
	auto rcv = mReqSocket.recv( req, zmq::recv_flags::none );
	if ( rcv.has_value() ) {
		for ( const auto &[ addr, _ ] : mAvailServers ) {
			spdlog::debug( "forward request `{}`to server `{}`", req.to_string(), addr );
			zmq::multipart_t forwardedReq;
			forwardedReq.push( std::move( req ) );
			forwardedReq.pushstr( addr );
			forwardedReq.send( mFrontSocket );
		}
	} else {
		spdlog::error( "failed to receive request!" );
	}
}

void FreelanceProxy::setupServers( const std::vector<uint16_t> &ports ) {
	for ( auto port : ports ) {
		auto addr = std::format( "tcp://localhost:{}", port );
		mAvailServers[ addr ] = std::chrono::steady_clock::now() + gServerHeartbeatExpiry;
		mRegisteredServers.push_back( addr );
	}
}

void FreelanceProxy::setupFrontSocket() {
	for ( const auto &[ addr, timepoint ] : mAvailServers ) {
		mFrontSocket.connect( addr );
	}

	mPoller.add( mFrontSocket, zmq::event_flags::pollin, [ this ]( zmq::event_flags flag ) { handleIncomingMessage(); } );
}

void FreelanceProxy::handleIncomingMessage() {
	zmq::multipart_t response;
	auto rcv = zmq::recv_multipart( mFrontSocket, std::back_inserter( response ) );
	if ( rcv.has_value() ) {
		ZmqUtil::dump( response, true );
		auto serverId = response.popstr();
		refreshHeartbeat( serverId );
		if ( response.size() == 1 && response.popstr() == "PONG" ) {
			// do nothing
			spdlog::debug( "received heartbeat from server={}", serverId );
		} else if ( response.size() == 2 ) {
			spdlog::info( "received response from a command request (serverId = {})", serverId );
			ZmqUtil::dump( response, true );
		}
	} else {
		spdlog::error( "failed to receive messages!" );
	}
}

void FreelanceProxy::setupHeartbeatSocket( const std::string &hbEndpoint ) {
	mHeartbeatSocket.bind( hbEndpoint );
	mPoller.add( mHeartbeatSocket, zmq::event_flags::pollin, [ this ]( zmq::event_flags flag ) { forwardHeartbeat(); } );
}

void FreelanceProxy::forwardHeartbeat() {
	zmq::message_t heartbeat;
	auto rcv = mHeartbeatSocket.recv( heartbeat, zmq::recv_flags::none );
	if ( rcv.has_value() ) {
		for ( const auto &addr : mRegisteredServers ) {
			spdlog::debug( "forwarding heartbeat to server={}...", addr );
			zmq::multipart_t message;
			message.pushstr( heartbeat.to_string() );
			message.pushstr( addr );

			ZmqUtil::dump( message, false );
			message.send( mFrontSocket );
		}
	} else {
		spdlog::error( "failed to receive heartbeat!" );
	}
}

void FreelanceProxy::refreshHeartbeat( const std::string &serverId ) {
	mAvailServers[ serverId ] = std::chrono::steady_clock::now() + gServerHeartbeatExpiry;
}

void FreelanceProxy::purgeDeadServers() {
	std::erase_if( mAvailServers, []( auto &&item ) { return std::chrono::steady_clock::now() > item.second; } );
}

void FreelanceProxy::run() {
	while ( 1 ) {
		[[maybe_unused]] auto _ = mPoller.wait( std::chrono::milliseconds( 100 ) );
		purgeDeadServers();
	}
}