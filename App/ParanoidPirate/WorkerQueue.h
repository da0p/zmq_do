#ifndef PARANOID_PIRATE_WORKER_QUEUE_H_
#define PARANOID_PIRATE_WORKER_QUEUE_H_
#include <chrono>
#include <deque>
#include <string>
#include <zmq.hpp>

struct Worker {
	std::string identity;
	std::chrono::steady_clock::time_point expiry;
};

class WorkerQueue {
  public:
	WorkerQueue();

	void add( const std::string &identity );

	void remove( const std::string &identity );

	void refresh( const std::string &identity );

	[[nodiscard]] bool isEmpty() const;

	[[nodiscard]] std::string pop();

	void purge();

	void sendHeartbeat( zmq::socket_t &socket );

	void clear();

  private:
	std::deque<Worker> mWorkers;
	std::chrono::steady_clock::time_point mDeadline;
};
#endif