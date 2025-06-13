#include "ZmqUtil.h"

#include <format>
#include <iostream>

#include <zmq.hpp>

namespace ZmqUtil {
	void sendString( zmq::socket_t &socket, std::string_view text, zmq::send_flags flag ) {
		zmq::message_t message{ text.length() };
		std::memcpy( message.data(), text.data(), text.length() );
		socket.send( message, flag );
	}

	void sendAllStrings( zmq::socket_t &socket, const std::vector<std::string> &frames ) {
		if ( frames.empty() ) {
			return;
		}

		if ( frames.size() == 1 ) {
			sendString( socket, frames.front() );
			return;
		}

		for ( size_t i = 0; i < frames.size() - 1; i++ ) {
			sendString( socket, frames[ i ], zmq::send_flags::sndmore );
		}

		sendString( socket, frames.back() );
	}

	std::optional<std::string> recvString( zmq::socket_t &socket, zmq::recv_flags flag ) {
		zmq::message_t message;
		auto rcv = socket.recv( message, flag );
		if ( rcv >= 0 ) {
			return message.to_string();
		}
		return {};
	}

	std::optional<std::vector<std::string>> recvAllStrings( zmq::socket_t &socket ) {
		std::vector<std::string> allMessages;
		while ( 1 ) {
			auto message = recvString( socket, zmq::recv_flags::none );
			if ( !message.has_value() ) {
				return {};
			}
			allMessages.push_back( message.value() );
			auto more = socket.get( zmq::sockopt::rcvmore );
			if ( !more ) {
				break;
			}
		}
		return allMessages;
	}

	void dump( zmq::socket_t &socket ) {
		while ( 1 ) {
			zmq::message_t message;
			auto size = socket.recv( message, zmq::recv_flags::none );
			if ( size < 0 ) {
				std::cerr << "Error in receiving message." << std::endl;
				return;
			}
			std::string raw( static_cast<char *>( message.data() ), size.value() );
			bool isText{ true };
			for ( size_t i = 0; i < size.value(); i++ ) {
				if ( raw[ i ] < 32 || raw[ i ] > 127 ) {
					isText = false;
					break;
				}
			}
			if ( isText ) {
				std::cout << std::format( "[{:<}]	{}\n", size.value(), raw );
			} else {
				auto out = std::format( "[{:<}]	", size.value() );
				for ( size_t i = 0; i < size.value(); i++ ) {
					out += std::format( "{0:2X}",
					                    static_cast<unsigned char>( raw[ i ] ) );
				}
				out += "\n";
				std::cout << out;
			}

			auto more = socket.get( zmq::sockopt::rcvmore );
			if ( !more ) {
				break;
			}
		}
	}
}