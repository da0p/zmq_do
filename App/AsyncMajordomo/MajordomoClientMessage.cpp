#include <MajordomoClientMessage.h>

namespace MajordomoClientMessage {
	Frame from( const std::string &message ) {
		return Frame{ message.begin(), message.end() };
	}

	std::optional<Request> Request::from( const Frames &frames ) {
		if ( frames.size() != 4 ) {
			return {};
		}

		if ( !frames[ 0 ].empty() ) {
			return {};
		}

		Request request;
		std::copy( frames[ 1 ].begin(), frames[ 1 ].end(), std::back_inserter( request.version ) );
		std::copy( frames[ 2 ].begin(), frames[ 2 ].end(), std::back_inserter( request.serviceName ) );
		request.body = frames[ 3 ];
		return request;
	}

	Frames Request::to( const Request &request ) {
		Frame frame_0;
		Frame frame_1{ request.version.begin(), request.version.end() };
		Frame frame_2{ request.serviceName.begin(), request.serviceName.end() };
		Frames frames{ frame_0, frame_1, frame_2, request.body };
		return frames;
	}

	std::optional<Reply> Reply::from( const Frames &frames ) {
		if ( frames.size() != 4 ) {
			return {};
		}

		Reply reply;
		std::copy( frames[ 1 ].begin(), frames[ 1 ].end(), std::back_inserter( reply.version ) );
		std::copy( frames[ 2 ].begin(), frames[ 2 ].end(), std::back_inserter( reply.serviceName ) );
		reply.body = frames[ 3 ];
		return reply;
	}

	Frames Reply::to( const Reply &reply ) {
		Frame frame_0;
		Frame frame_1{ reply.version.begin(), reply.version.end() };
		Frame frame_2{ reply.serviceName.begin(), reply.serviceName.end() };
		Frames frames{ frame_0, frame_1, frame_2, reply.body };
		return frames;
	}

	Frames Reply::to( const Reply &reply, const std::string &clientAddr ) {
		Frame frame_0{ clientAddr.begin(), clientAddr.end() };
		Frame frame_1;
		Frame frame_2{ reply.version.begin(), reply.version.end() };
		Frame frame_3{ reply.serviceName.begin(), reply.serviceName.end() };
		Frames frames{ frame_0, frame_1, frame_2, frame_3, reply.body };
		return frames;
	}
};