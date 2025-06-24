#include <BinaryStarClient.h>

int main() {
	BinaryStarClient client{"tcp://localhost:5001", "tcp://localhost:5002"};
	client.run();

	return 0;
}