#include <zmq.hpp>
#include <spdlog/spdlog.h>

int main(int argc, char *argv[])
{
    zmq::context_t zmqContext;
    zmq::socket_t zmqSocket{zmqContext, zmq::socket_type::req};

    spdlog::info("Connecting to hello world server");
    zmqSocket.connect("tcp://localhost:5555");

    constexpr std::string_view reqMsg = "Hello";
    zmq::message_t request{reqMsg.length()};
    std::memcpy(request.data(), reqMsg.data(), reqMsg.length());
    spdlog::info("Sending Hello to server...");
    zmqSocket.send(request, zmq::send_flags::none);

    zmq::message_t reply;
    auto result = zmqSocket.recv(reply, zmq::recv_flags::none);
    if (result.has_value()) {
        std::string repData{static_cast<char*>(reply.data()), result.value()};
        spdlog::info("Receiving {} bytes. Data = {}", result.value(), repData);
    }
}