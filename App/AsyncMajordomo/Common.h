#ifndef ASYNC_MAJORDOMO_COMMON_H_
#define ASYNC_MAJORDOMO_COMMON_H_

#include <chrono>

constexpr auto gMajVer = "MDPC01";
constexpr auto gWorkerHeartbeatExpiryDuration = std::chrono::seconds( 3 );
constexpr auto gBrokerHeartbeatInterval = std::chrono::seconds( 1 );
constexpr auto gDiscoveryService = "mmi.service";
#endif