# Paranoid Pirate

- The worker can detect failure from the queue using heartbeat sent from the queue
- The queue can detect failure from the worker using heartbeat sent from the worker, and then remove the corresponding worker