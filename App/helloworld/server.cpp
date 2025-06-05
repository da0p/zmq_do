#include <cstring>
#include <string>
#include <sstream>


#include <zmq.hpp>
#include <spdlog/spdlog.h>

// This example demonstrates Request - Reply socket pair

namespace {
    // size of zmq thread pool to handle I/O operations. If the application contains
    // only inproc, then it can be 0, otherwise, it should be at least 1 
    const int32_t numberOfThreads = 2;    
}

int main( int argc, char *argv[] ) {
    zmq::context_t zmqContext{numberOfThreads};
    zmq::socket_t zmqSocket{zmqContext, zmq::socket_type::rep};
    // bind the socket to port 5555
    zmqSocket.bind("tcp://*:5555");

    while (true) {
        zmq::message_t request;

        // wait for next request from client;
        auto result = zmqSocket.recv(request, zmq::recv_flags::none);
        if (result.has_value()) {
            std::string reqData{static_cast<char*>(request.data()), result.value()};
            spdlog::info("Received {} bytes, request = {}", result.value(), reqData);
        }
        constexpr std::string_view response{"World"};
        zmq::message_t reply{response.length()};
        std::memcpy(reply.data(), response.data(), response.length());
        zmqSocket.send(reply, zmq::send_flags::none);
    }
}