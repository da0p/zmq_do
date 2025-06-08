#include <thread>

#include <spdlog/spdlog.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>

void step1( zmq::context_t &ctx ) {
	zmq::socket_t xmitter{ ctx, zmq::socket_type::pair };
	xmitter.connect( "inproc://step2" );

	spdlog::info( "Step 1 ready, signaling step 2" );

	zmq::message_t ready( std::string( "Ready" ) );
	xmitter.send( ready, zmq::send_flags::none );
}

void step2( zmq::context_t &ctx ) {
	zmq::socket_t receiver{ ctx, zmq::socket_type::pair };
	receiver.bind( "inproc://step2" );

	std::thread thd{ step1, std::ref( ctx ) };

	zmq::message_t ready;
	[[maybe_unused]] auto _ = receiver.recv( ready, zmq::recv_flags::none );

	zmq::socket_t xmitter{ ctx, zmq::socket_type::pair };
	xmitter.connect( "inproc://step3" );

	spdlog::info( "Step 2 ready, signaling step 3" );

	ready.rebuild( std::string( "Ready" ) );
	xmitter.send( ready, zmq::send_flags::none );
	thd.join();
}

int main( int argc, char *argv[] ) {
	zmq::context_t ctx{ 1 };

	// bind inproc socket before starting step 2
	// If inproc and socket pairs are used only when low latency is really vital.
	// Because it tightly bounds applications
	// Note that PAIR refuses more than one connection
	zmq::socket_t receiver{ ctx, zmq::socket_type::pair };
	receiver.bind( "inproc://step3" );

	std::thread thd{ step2, std::ref( ctx ) };

	zmq::message_t ready;
	[[maybe_unused]] auto _ = receiver.recv( ready, zmq::recv_flags::none );
	thd.join();

	return 0;
}