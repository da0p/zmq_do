#ifndef ASYNC_MAJORDOMO_CLIENT_MESSAGE_H_
#define ASYNC_MAJORDOMO_CLIENT_MESSAGE_H_

#include <cstdint>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

namespace MajordomoClientMessage {
	using Frame = std::vector<uint8_t>;
	using Frames = std::vector<Frame>;

	struct BaseCmd {
		std::string version;
		std::string serviceName;
		Frame body;
	};

	inline Frame from( const std::string &message ) {
		return Frame{ message.begin(), message.end() };
	}

	struct Request : public BaseCmd {
		static inline std::optional<Request> from( const Frames &frames ) {
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

		static inline Frames to( const Request &request ) {
			Frame frame_0;
			Frame frame_1{ request.version.begin(), request.version.end() };
			Frame frame_2{ request.serviceName.begin(), request.serviceName.end() };
			Frames frames{ frame_0, frame_1, frame_2, request.body };
			return frames;
		}
	};

	struct Reply : BaseCmd {
		static inline std::optional<Reply> from( const Frames &frames ) {
			if ( frames.size() != 4 ) {
				return {};
			}

			Reply reply;
			std::copy( frames[ 1 ].begin(), frames[ 1 ].end(), std::back_inserter( reply.version ) );
			std::copy( frames[ 2 ].begin(), frames[ 2 ].end(), std::back_inserter( reply.serviceName ) );
			reply.body = frames[ 3 ];

			return reply;
		}

		static inline Frames to( const Reply &reply ) {
			Frame frame_0;
			Frame frame_1{ reply.version.begin(), reply.version.end() };
			Frame frame_2{ reply.serviceName.begin(), reply.serviceName.end() };
			Frames frames{ frame_0, frame_1, frame_2, reply.body };
			return frames;
		}

		static inline Frames to( const Reply &reply, const std::string &clientAddr ) {
			Frame frame_0{ clientAddr.begin(), clientAddr.end() };
			Frame frame_1;
			Frame frame_2{ reply.version.begin(), reply.version.end() };
			Frame frame_3{ reply.serviceName.begin(), reply.serviceName.end() };
			Frames frames{ frame_0, frame_1, frame_2, frame_3, reply.body };
			return frames;
		}
	};
}
#endif