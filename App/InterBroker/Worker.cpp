#include <Worker.h>

#include <ZmqUtil.h>

Worker::Worker( zmq::context_t &ctx, const std::string &identity, uint32_t num ) :
        mIdentity{ identity }, mNum{ num }, mSocket{ ctx, zmq::socket_type::req } {
}

void Worker::run() {
	mIsRunning = true;
	mRunningThread = std::thread( &Worker::doWork, this );
}

void Worker::stop() {
	mIsRunning = false;
	mRunningThread.join();
}

void Worker::doWork() {
	mSocket.set( zmq::sockopt::routing_id, std::format( "worker-{}-{}", mIdentity, mNum ) );
	auto url = std::format( "ipc://{}-localbe.ipc", mIdentity );
	mSocket.connect( url );

	ZmqUtil::sendString( mSocket, "ready" );

	while ( mIsRunning ) {
		auto frames = ZmqUtil::recvAllStrings( mSocket );
		if ( frames.has_value() ) {
			auto allFrames = frames.value();
			allFrames.pop_back();
			allFrames.push_back( "ok" );
			ZmqUtil::sendAllStrings( mSocket, allFrames );
		}
	}
}