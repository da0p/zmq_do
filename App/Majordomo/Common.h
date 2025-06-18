#ifndef MAJORDOMO_COMMON_H_
#define MAJORDOMO_COMMON_H_

#include <cstdint>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

#include <zmq.hpp>

constexpr auto gMajVer = "MDPC01";

enum class MessageType : uint8_t {
	Ready = 0x01,
	Request = 0x02,
	Reply = 0x03,
	Heartbeat = 0x04,
	Disconnect = 0x05,
};

struct Ready {
	std::string version;
	std::string serviceName;
};

struct Request {
	std::string version;
	std::string clientAddr;
	std::vector<uint8_t> body;
};

struct Reply {
	std::string version;
	std::string clientAddr;
	std::vector<uint8_t> body;
};

struct Heartbeat {
	std::string version;
};

struct Disconnect {
	std::string version;
};

[[nodiscard]] inline std::vector<uint8_t> fromStr( const std::string &message ) {
	std::vector<uint8_t> frame{ message.begin(), message.end() };
	return frame;
}

[[nodiscard]] inline std::vector<std::vector<uint8_t>> toFrames( const Ready &ready ) {
	std::vector<uint8_t> frame_0;
	std::vector<uint8_t> frame_1{ ready.version.begin(), ready.version.end() };
	std::vector<uint8_t> frame_2{ static_cast<uint8_t>( MessageType::Ready ) };
	std::vector<uint8_t> frame_3{ ready.serviceName.begin(), ready.serviceName.end() };
	std::vector<std::vector<uint8_t>> frames{ frame_0, frame_1, frame_2, frame_3 };
	return frames;
}

[[nodiscard]] inline std::vector<std::vector<uint8_t>> toFrames( const Request &request ) {
	std::vector<uint8_t> frame_0;
	std::vector<uint8_t> frame_1{ request.version.begin(), request.version.end() };
	std::vector<uint8_t> frame_2{ static_cast<uint8_t>( MessageType::Request ) };
	std::vector<uint8_t> frame_3{ request.clientAddr.begin(), request.clientAddr.end() };
	std::vector<uint8_t> frame_4;
	std::vector<std::vector<uint8_t>> frames{ frame_0, frame_1, frame_2, frame_3, frame_4, request.body };
	return frames;
}

[[nodiscard]] inline std::vector<std::vector<uint8_t>> toFrames( const Reply &reply ) {
	std::vector<uint8_t> frame_0;
	std::vector<uint8_t> frame_1{ reply.version.begin(), reply.version.end() };
	std::vector<uint8_t> frame_2{ static_cast<uint8_t>( MessageType::Reply ) };
	std::vector<uint8_t> frame_3{ reply.clientAddr.begin(), reply.clientAddr.end() };
	std::vector<uint8_t> frame_4;
	std::vector<std::vector<uint8_t>> frames{ frame_0, frame_1, frame_2, frame_3, frame_4, reply.body };
	return frames;
}

[[nodiscard]] inline std::vector<std::vector<uint8_t>> toFrames( const Heartbeat &heartbeat ) {
	std::vector<uint8_t> frame_0;
	std::vector<uint8_t> frame_1{ heartbeat.version.begin(), heartbeat.version.end() };
	std::vector<uint8_t> frame_2{ static_cast<uint8_t>( MessageType::Heartbeat ) };
	std::vector<std::vector<uint8_t>> frames{ frame_0, frame_1, frame_2 };
	return frames;
}

[[nodiscard]] inline std::vector<std::vector<uint8_t>> toFrames( const Disconnect &disconnect ) {
	std::vector<uint8_t> frame_0;
	std::vector<uint8_t> frame_1{ disconnect.version.begin(), disconnect.version.end() };
	std::vector<uint8_t> frame_2{ static_cast<uint8_t>( MessageType::Disconnect ) };
	std::vector<std::vector<uint8_t>> frames{ frame_0, frame_1, frame_2 };
	return frames;
}

[[nodiscard]] inline std::optional<Ready> toReady( const std::vector<std::vector<uint8_t>> &frames ) {
	if ( frames.size() != 4 ) {
		return {};
	}

	if ( !frames[ 0 ].empty() ) {
		return {};
	}

	if ( frames[ 2 ].size() != 1 || frames[ 2 ].front() != static_cast<uint8_t>( MessageType::Ready ) ) {
		return {};
	}

	Ready ready;
	std::copy( frames[ 1 ].begin(), frames[ 1 ].end(), std::back_inserter( ready.version ) );
	std::copy( frames[ 3 ].begin(), frames[ 3 ].end(), std::back_inserter( ready.serviceName ) );

	return ready;
}

[[nodiscard]] inline std::optional<Request> toRequest( const std::vector<std::vector<uint8_t>> &frames ) {
	if ( frames.size() != 6 ) {
		return {};
	}

	if ( !frames[ 0 ].empty() ) {
		return {};
	}

	if ( frames[ 2 ].size() != 1 || frames[ 2 ].front() != static_cast<uint8_t>( MessageType::Request ) ) {
		return {};
	}

	if ( !frames[ 4 ].empty() ) {
		return {};
	}

	Request request;
	std::copy( frames[ 1 ].begin(), frames[ 1 ].end(), std::back_inserter( request.version ) );
	std::copy( frames[ 3 ].begin(), frames[ 3 ].end(), std::back_inserter( request.clientAddr ) );
	request.body = frames[ 5 ];

	return request;
}

[[nodiscard]] inline std::optional<Reply> toReply( const std::vector<std::vector<uint8_t>> &frames ) {
	if ( frames.size() != 6 ) {
		return {};
	}

	if ( !frames[ 0 ].empty() ) {
		return {};
	}

	if ( frames[ 2 ].size() != 1 || frames[ 2 ].front() != static_cast<uint8_t>( MessageType::Reply ) ) {
		return {};
	}

	Reply reply;
	std::copy( frames[ 1 ].begin(), frames[ 1 ].end(), std::back_inserter( reply.version ) );
	std::copy( frames[ 3 ].begin(), frames[ 3 ].end(), std::back_inserter( reply.clientAddr ) );
	reply.body = frames[ 5 ];
	return reply;
}

[[nodiscard]] inline std::optional<Heartbeat> toHeartbeat( const std::vector<std::vector<uint8_t>> &frames ) {
	if ( frames.size() != 3 ) {
		return {};
	}

	if ( !frames[ 0 ].empty() ) {
		return {};
	}

	if ( frames[ 2 ].size() != 1 || frames[ 2 ].front() != static_cast<uint8_t>( MessageType::Heartbeat ) ) {
		return {};
	}

	Heartbeat heartbeat;
	std::copy( frames[ 1 ].begin(), frames[ 1 ].end(), std::back_inserter( heartbeat.version ) );
	return heartbeat;
}

[[nodiscard]] inline std::optional<Disconnect> toDisconnect( const std::vector<std::vector<uint8_t>> &frames ) {
	if ( frames.size() != 3 ) {
		return {};
	}

	if ( !frames[ 0 ].empty() ) {
		return {};
	}

	if ( frames[ 2 ].size() != 1 || frames[ 2 ].front() != static_cast<uint8_t>( MessageType::Disconnect ) ) {
		return {};
	}

	Disconnect disconnect;
	std::copy( frames[ 1 ].begin(), frames[ 1 ].end(), std::back_inserter( disconnect.version ) );
	return disconnect;
}

#endif