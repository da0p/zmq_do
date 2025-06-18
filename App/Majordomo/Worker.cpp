#include <MajordomoWorker.h>

int main( int argc, char *argv[] ) {
	MajordomoWorker maj{ "tcp://localhost:5555", "echo", std::chrono::seconds( 1 ) };
	maj.receive();
	return 0;
}