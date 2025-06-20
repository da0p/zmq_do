#include "ZmqUtil.h"

#include <format>
#include <iostream>

#include <zmq.hpp>

namespace ZmqUtil {
	void sendString( zmq::socket_t &socket, std::string_view text, zmq::send_flags flag ) {
		std::vector<uint8_t> frame{ text.begin(), text.end() };
		sendFrame( socket, frame, flag );
	}

	void sendAllStrings( zmq::socket_t &socket, const std::vector<std::string> &frames ) {
		std::vector<std::vector<uint8_t>> allFrames;
		for ( size_t i = 0; i < frames.size(); i++ ) {
			std::vector<uint8_t> frame{ frames[ i ].begin(), frames[ i ].end() };
			allFrames.push_back( frame );
		}
		sendAllFrames( socket, allFrames );
	}

	void sendFrame( zmq::socket_t &socket, const std::vector<uint8_t> &frame, zmq::send_flags flag ) {
		zmq::message_t message{ frame.size() };
		std::memcpy( message.data(), frame.data(), frame.size() );
		socket.send( message, flag );
	}

	void sendAllFrames( zmq::socket_t &socket, const std::vector<std::vector<uint8_t>> &frames ) {
		if ( frames.empty() ) {
			return;
		}

		if ( frames.size() == 1 ) {
			sendFrame( socket, frames.front() );
			return;
		}

		for ( size_t i = 0; i < frames.size() - 1; i++ ) {
			sendFrame( socket, frames[ i ], zmq::send_flags::sndmore );
		}
		sendFrame( socket, frames.back() );
	}

	std::optional<std::string> recvString( zmq::socket_t &socket, zmq::recv_flags flag ) {
		zmq::message_t message;
		auto rcv = socket.recv( message, flag );
		if ( rcv >= 0 ) {
			return message.to_string();
		}
		return {};
	}

	std::optional<std::vector<uint8_t>> recvFrame( zmq::socket_t &socket, zmq::recv_flags flag ) {
		zmq::message_t message;
		auto rcv = socket.recv( message, flag );
		if ( rcv >= 0 ) {
			std::vector<uint8_t> frame{ static_cast<uint8_t *>( message.data() ),
				                        static_cast<uint8_t *>( message.data() ) + rcv.value() };
			return frame;
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

	std::optional<std::vector<std::vector<uint8_t>>> recvAllFrames( zmq::socket_t &socket ) {
		std::vector<std::vector<uint8_t>> allFrames;
		while ( 1 ) {
			auto frame = recvFrame( socket, zmq::recv_flags::none );
			if ( !frame.has_value() ) {
				return {};
			}
			allFrames.push_back( frame.value() );
			auto more = socket.get( zmq::sockopt::rcvmore );
			if ( !more ) {
				break;
			}
		}
		return allFrames;
	}

	void dump( zmq::socket_t &socket ) {
		while ( 1 ) {
			zmq::message_t message;
			auto size = socket.recv( message, zmq::recv_flags::none );
			if ( size < 0 ) {
				std::cerr << "Error in receiving message." << std::endl;
				return;
			}
			std::vector<uint8_t> frame{ static_cast<uint8_t *>( message.data() ),
				                        static_cast<uint8_t *>( message.data() ) + size.value() };
			dump( frame );
			auto more = socket.get( zmq::sockopt::rcvmore );
			if ( !more ) {
				break;
			}
		}
	}

	void dump( const std::vector<std::vector<uint8_t>> &frames ) {
		for ( const auto &frame : frames ) {
			dump( frame );
		}
	}

	void dump( const std::vector<uint8_t> &frame ) {
		std::string raw( frame.begin(), frame.end() );
		bool isText{ true };
		for ( size_t i = 0; i < frame.size(); i++ ) {
			if ( raw[ i ] < 32 || raw[ i ] > 127 ) {
				isText = false;
				break;
			}
		}
		if ( isText ) {
			std::cout << std::format( "[{:<}]	{}\n", frame.size(), raw );
		} else {
			auto out = std::format( "[{:<}]	", frame.size() );
			for ( size_t i = 0; i < frame.size(); i++ ) {
				out += std::format( "{0:2X}", static_cast<unsigned char>( raw[ i ] ) );
			}
			out += "\n";
			std::cout << out;
		}
	}

	void dump( const std::vector<std::string> &messages ) {
		for ( const auto &message : messages ) {
			std::vector<uint8_t> frame{ message.begin(), message.end() };
			dump( frame );
		}
	}
}