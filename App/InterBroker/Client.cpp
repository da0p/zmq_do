#include <Client.h>

#include <thread>

#include <spdlog/spdlog.h>

#include <ZmqUtil.h>

Client::Client( zmq::context_t &ctx, const std::string &identity, uint32_t num ) :
        mIdentity{ identity },
        mNum{ num },
        mSocket{ ctx, zmq::socket_type::req }

{
}

void Client::run() {
	mIsRunning = true;
	mRunningThread = std::thread( &Client::doRequest, this );
}

void Client::stop() {
	mIsRunning = false;
	mRunningThread.join();
}

void Client::doRequest() {
	auto url = std::format( "ipc://{}-localfe.ipc", mIdentity );
	mSocket.set( zmq::sockopt::routing_id, std::format( "client-{}", mNum ) );
	mSocket.connect( url );
	while ( mIsRunning ) {
		spdlog::info( "Client `{}-{}` send `hello`", mIdentity, mNum );
		ZmqUtil::sendString( mSocket, "hello" );
		auto reply = ZmqUtil::recvString( mSocket );
		if ( reply.has_value() ) {
			spdlog::info( "Client `{}-{}` receive: {}", mIdentity, mNum, reply.value() );
		}
		std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
	}
}