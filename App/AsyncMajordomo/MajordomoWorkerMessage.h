#ifndef ASYNC_MAJORDOMO_WORKER_MESSAGE_H_
#define ASYNC_MAJORDOMO_WORKER_MESSAGE_H_
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <Common.h>

namespace MajordomoWorkerMessage {
	using Frame = std::vector<uint8_t>;
	using Frames = std::vector<Frame>;

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

		static std::optional<Ready> from( const Frames &frames );

		static Frames to( const Ready &ready );
	};

	struct Request {
		std::string version;
		std::string clientAddr;
		Frame body;

		static std::optional<Request> from( const Frames &frames );

		static Frames to( const Request &request );
		static Frames to( const Request &request, const std::string &workerIdentity );
	};

	struct Reply {
		std::string version;
		std::string clientAddr;
		Frame body;

		static std::optional<Reply> from( const Frames &frames );
		static Frames to( const Reply &reply );
	};

	struct Heartbeat {
		std::string version;

		static std::optional<Heartbeat> from( const Frames &frames );
		static Frames to( const Heartbeat &heartbeat );
		static Frames to( const Heartbeat &heartbeat, const std::string &workerIdentity );
	};

	struct Disconnect {
		std::string version;

		static std::optional<Disconnect> from( const Frames &frames );
		static Frames to( const Disconnect &disconnect );
		static Frames to( const Disconnect &disconnect, const std::string &workerIdentity );
	};
}

#endif