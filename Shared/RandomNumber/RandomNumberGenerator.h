#ifndef RANDOM_NUMBER_GENERATOR_H_
#define RANDOM_NUMBER_GENERATOR_H_

#include <random>

class RandomNumberGenerator {
  public:
	explicit RandomNumberGenerator( int32_t start, int32_t end );

	[[nodiscard]] int32_t generate();

  private:
	std::random_device mDevice;
	std::mt19937 mRandom;
	std::uniform_int_distribution<std::mt19937::result_type> mDistribution;
};
#endif