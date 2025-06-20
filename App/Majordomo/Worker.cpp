#include <MajordomoWorker.h>
#include <spdlog/spdlog.h>

int main( int argc, char *argv[] ) {
	spdlog::set_level( spdlog::level::err );
	MajordomoWorker maj{ "tcp://localhost:5556", "echo", std::chrono::seconds( 1 ) };
	maj.receive();
	return 0;
}