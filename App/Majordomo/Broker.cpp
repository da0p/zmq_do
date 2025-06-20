#include <MajordomoBroker.h>

int main( int argc, char *argv[] ) {
	MajordomoBroker maj{ "tcp://*:5555", "tcp://*:5556" };

	maj.run();

	return 0;
}