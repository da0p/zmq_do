#include "ZmqUtil.h"

#include <zmq.hpp>

namespace ZmqUtil {
	void sendString( zmq::socket_t &socket, std::string_view text, zmq::send_flags flag ) {
		zmq::message_t message{ text.length() };
		std::memcpy( message.data(), text.data(), text.length() );
		socket.send( message, flag );
	}

	std::string recvString( zmq::socket_t &socket, zmq::recv_flags flag ) {
		zmq::message_t message;
		[[maybe_unused]] auto _ = socket.recv( message, flag );
		return message.to_string();
	}
}