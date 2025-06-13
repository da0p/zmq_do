#ifndef ZMQ_UTIL_H_
#define ZMQ_UTIL_H_

#include <optional>
#include <string>
#include <vector>

#include <zmq.hpp>

namespace ZmqUtil {
	void sendString( zmq::socket_t &socket,
	                 std::string_view text,
	                 zmq::send_flags flag = zmq::send_flags::none );
	void sendAllStrings( zmq::socket_t &socket, const std::vector<std::string> &frames );
	[[nodiscard]] std::optional<std::string> recvString(
	  zmq::socket_t &socket,
	  zmq::recv_flags flag = zmq::recv_flags::none );
	[[nodiscard]] std::optional<std::vector<std::string>> recvAllStrings( zmq::socket_t &socket );
	void dump( zmq::socket_t &socket );
};

#endif