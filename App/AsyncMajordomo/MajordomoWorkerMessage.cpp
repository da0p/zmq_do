#include <MajordomoWorkerMessage.h>

namespace MajordomoWorkerMessage {
	std::optional<Ready> Ready::from( const Frames &frames ) {
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

	Frames Ready::to( const Ready &ready ) {
		Frame frame_0;
		Frame frame_1{ ready.version.begin(), ready.version.end() };
		Frame frame_2{ static_cast<uint8_t>( MessageType::Ready ) };
		Frame frame_3{ ready.serviceName.begin(), ready.serviceName.end() };
		Frames frames{ frame_0, frame_1, frame_2, frame_3 };
		return frames;
	}

	std::optional<Request> Request::from( const Frames &frames ) {
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

	Frames Request::to( const Request &request ) {
		Frame frame_0;
		Frame frame_1{ request.version.begin(), request.version.end() };
		Frame frame_2{ static_cast<uint8_t>( MessageType::Request ) };
		Frame frame_3{ request.clientAddr.begin(), request.clientAddr.end() };
		Frame frame_4;
		Frames frames{ frame_0, frame_1, frame_2, frame_3, frame_4, request.body };
		return frames;
	}

	Frames Request::to( const Request &request, const std::string &workerIdentity ) {
		Frame frame_0{ workerIdentity.begin(), workerIdentity.end() };
		Frame frame_1;
		Frame frame_2{ request.version.begin(), request.version.end() };
		Frame frame_3{ static_cast<uint8_t>( MessageType::Request ) };
		Frame frame_4{ request.clientAddr.begin(), request.clientAddr.end() };
		Frame frame_5;
		Frames frames{ frame_0, frame_1, frame_2, frame_3, frame_4, frame_5, request.body };
		return frames;
	}

	std::optional<Reply> Reply::from( const Frames &frames ) {
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

	Frames Reply::to( const Reply &reply ) {
		Frame frame_0;
		Frame frame_1{ reply.version.begin(), reply.version.end() };
		Frame frame_2{ static_cast<uint8_t>( MessageType::Reply ) };
		Frame frame_3{ reply.clientAddr.begin(), reply.clientAddr.end() };
		Frame frame_4;
		Frames frames{ frame_0, frame_1, frame_2, frame_3, frame_4, reply.body };
		return frames;
	}

	std::optional<Heartbeat> Heartbeat::from( const Frames &frames ) {
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

	Frames Heartbeat::to( const Heartbeat &heartbeat ) {
		Frame frame_0;
		Frame frame_1{ heartbeat.version.begin(), heartbeat.version.end() };
		Frame frame_2{ static_cast<uint8_t>( MessageType::Heartbeat ) };
		Frames frames{ frame_0, frame_1, frame_2 };
		return frames;
	}

	Frames Heartbeat::to( const Heartbeat &heartbeat, const std::string &workerIdentity ) {
		Frame frame_0{ workerIdentity.begin(), workerIdentity.end() };
		Frame frame_1;
		Frame frame_2{ heartbeat.version.begin(), heartbeat.version.end() };
		Frame frame_3{ static_cast<uint8_t>( MessageType::Heartbeat ) };
		Frames frames{ frame_0, frame_1, frame_2, frame_3 };
		return frames;
	}

	std::optional<Disconnect> Disconnect::from( const Frames &frames ) {
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

	Frames Disconnect::to( const Disconnect &disconnect ) {
		Frame frame_0;
		Frame frame_1{ disconnect.version.begin(), disconnect.version.end() };
		Frame frame_2{ static_cast<uint8_t>( MessageType::Disconnect ) };
		Frames frames{ frame_0, frame_1, frame_2 };
		return frames;
	}

	Frames Disconnect::to( const Disconnect &disconnect, const std::string &workerIdentity ) {
		Frame frame_0{ workerIdentity.begin(), workerIdentity.end() };
		Frame frame_1;
		Frame frame_2{ disconnect.version.begin(), disconnect.version.end() };
		Frame frame_3{ static_cast<uint8_t>( MessageType::Disconnect ) };
		Frames frames{ frame_0, frame_1, frame_2, frame_3 };
		return frames;
	}
}