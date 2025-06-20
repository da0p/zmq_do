#include <MajordomoBroker.h>

#include <spdlog/spdlog.h>

int main( int argc, char *argv[] ) {
	spdlog::set_level( spdlog::level::err );
	MajordomoBroker maj{ "tcp://*:5555", "tcp://*:5556" };

	maj.run();

	return 0;
}