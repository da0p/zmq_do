#include <RequestService.h>

#include <array>
#include <thread>

RequestService::RequestService( zmq::context_t &ctx, const std::string &endpoint, std::chrono::milliseconds interval ) :
        mSocket{ ctx, zmq::socket_type::pair }, mInterval{ interval } {
	setup( endpoint );
}

void RequestService::setup( const std::string &endpoint ) {
	mSocket.connect( endpoint );
}

void RequestService::run() {
	std::array<std::string, 3> commands{ "Hello", "Guten Morgen", "Bonjour" };
	size_t i = 0;
	while ( 1 ) {
		auto command = commands[ i % 3 ];
		zmq::message_t message{ command };
		mSocket.send( message, zmq::send_flags::none );
		std::this_thread::sleep_for( mInterval );
		i++;
	}
}