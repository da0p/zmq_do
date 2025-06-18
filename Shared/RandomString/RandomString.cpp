#include "RandomString.h"
#include <RandomNumberGenerator.h>

namespace RandomString {
	std::string generate( uint32_t length ) {
		std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
		RandomNumberGenerator rnd{ 0, static_cast<int32_t>( chars.length() ) };
		std::string randomStr;
		for ( size_t i = 0; i < length; i++ ) {
			randomStr += chars[ rnd.generate() ];
		}
		return randomStr;
	}
}