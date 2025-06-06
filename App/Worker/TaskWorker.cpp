#include <chrono>
#include <thread>

#include <zmq.hpp>
#include <spdlog/spdlog.h>

int main(int argc, char *argv[])
{
    zmq::context_t zmqContext;

    // socket to receive messages on
    zmq::socket_t zmqPull{zmqContext, zmq::socket_type::pull};
    zmqPull.connect("tcp://localhost:5557");

    // socket to send messages to
    zmq::socket_t zmqPush{zmqContext, zmq::socket_type::push};
    zmqPush.connect("tcp://localhost:5558");

    while (1) {
        zmq::message_t message;
        auto result = zmqPull.recv(message, zmq::recv_flags::none);
        if (result.has_value()) {
            std::string workLoadStr(static_cast<char*>(message.data()), result.value());
            int32_t workLoad = std::stoi(workLoadStr);
            spdlog::info("Receiving workLoad = {} ms", workLoad);

            // assume do some work
            std::this_thread::sleep_for(std::chrono::milliseconds(workLoad));

            // send the result
            message.rebuild();
            zmqPush.send(message, zmq::send_flags::none);
        }
    }

    return 0;
}