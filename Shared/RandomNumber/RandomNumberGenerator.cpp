#include "RandomNumberGenerator.h"

RandomNumberGenerator::RandomNumberGenerator( int32_t start, int32_t end ) :
        mRandom{ mDevice() }, mDistribution( start, end ) {
}

int32_t RandomNumberGenerator::generate() {
	return mDistribution( mRandom );
}