#ifndef ZMQ_UTIL_H_
#define ZMQ_UTIL_H_

#include <zmq.hpp>

namespace ZmqUtil {
	void sendString( zmq::socket_t &socket, std::string_view text, zmq::send_flags flag = zmq::send_flags::none );
	[[nodiscard]] std::string recvString( zmq::socket_t &socket, zmq::recv_flags flag = zmq::recv_flags::none );
	void dump( zmq::socket_t &socket );
};

#endif