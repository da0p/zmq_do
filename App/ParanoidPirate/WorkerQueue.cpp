#include "WorkerQueue.h"
#include "ZmqUtil.h"

#include <chrono>
#include <deque>

#include <spdlog/spdlog.h>

namespace {
	constexpr auto gExpiryDuration = std::chrono::seconds( 3 );
}

WorkerQueue::WorkerQueue() : mDeadline{ std::chrono::steady_clock::now() } {
}

void WorkerQueue::add( const std::string &identity ) {
	auto found = std::any_of(
	  mWorkers.begin(), mWorkers.end(), [ &identity ]( auto &&worker ) { return identity == worker.identity; } );
	if ( found ) {
		spdlog::warn( "Duplicate worker identity!" );
	} else {
		mWorkers.emplace_back( identity, std::chrono::steady_clock::now() + gExpiryDuration );
	}
}

void WorkerQueue::remove( const std::string &identity ) {
	std::erase_if( mWorkers, [ &identity ]( auto &&worker ) { return worker.identity == identity; } );
}

void WorkerQueue::refresh( const std::string &identity ) {
	for ( auto &[ id, expiry ] : mWorkers ) {
		if ( id == identity ) {
			expiry = std::chrono::steady_clock::now() + gExpiryDuration;
			spdlog::debug( "Change expiry of {} to {}", identity, expiry.time_since_epoch().count() );
			break;
		}
	}
}

bool WorkerQueue::isEmpty() const {
	return mWorkers.empty();
}

std::string WorkerQueue::pop() {
	auto worker = mWorkers.front();
	mWorkers.pop_front();
	return worker.identity;
}

void WorkerQueue::purge() {
	std::erase_if( mWorkers, []( auto &&worker ) {
		if ( std::chrono::steady_clock::now() > worker.expiry ) {
			spdlog::debug(
			  "worker {} heartbeat expired, expiry = {}", worker.identity, worker.expiry.time_since_epoch().count() );
			return true;
		}
		return false;
	} );
}

void WorkerQueue::sendHeartbeat( zmq::socket_t &socket ) {
	auto now = std::chrono::steady_clock::now();
	if ( now > mDeadline ) {
		auto logInfo = std::format( "sendHeartbeat to [" );

		std::for_each( mWorkers.begin(), mWorkers.end(), [ &socket, &logInfo ]( auto &&worker ) {
			logInfo += worker.identity + ", ";
			std::vector<std::string> heartbeat{ worker.identity, "heartbeat" };
			ZmqUtil::sendAllStrings( socket, heartbeat );
		} );
		logInfo += "]";
		spdlog::debug( "{}", logInfo );
		mDeadline = now + gExpiryDuration;
	}
}

void WorkerQueue::clear() {
	mWorkers.clear();
}