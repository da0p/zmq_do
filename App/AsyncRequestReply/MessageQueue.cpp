#include <zmq.hpp>
#include <zmq_addon.hpp>

int main(int argc, char *argv[])
{
    zmq::context_t ctx;
    
    zmq::socket_t front{ctx, zmq::socket_type::router};
    front.bind("tcp://*:5559");

    zmq::socket_t back{ctx, zmq::socket_type::dealer};
    back.bind("tcp://*:5560");

    // start the proxy
    zmq::proxy(front, back);

    return 0;
}