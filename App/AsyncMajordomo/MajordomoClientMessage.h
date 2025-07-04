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

	Frame from( const std::string &message );

	struct Request : public BaseCmd {
		static std::optional<Request> from( const Frames &frames );
		static Frames to( const Request &request );
	};

	struct Reply : BaseCmd {
		static std::optional<Reply> from( const Frames &frames );
		static Frames to( const Reply &reply );
		static Frames to( const Reply &reply, const std::string &clientAddr );
	};

	struct DiscoveryRequest {
		std::string header;
		std::string serviceName;

		static std::optional<DiscoveryRequest> from( const Frames &frames );
		static Frames to( const DiscoveryRequest &discovery );
	};

	struct DiscoveryReply {
		std::string header;
		std::string serviceName;
		std::string status;

		static std::optional<DiscoveryReply> from( const Frames &frames );
		static Frames to( const DiscoveryReply &discovery, const std::string &clientAddr );
	};
}
#endif