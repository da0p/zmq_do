# SimplePirate Pattern

- Client-side relibility by retrying
- Load-balancing and fail-over
- Not robust if the queue crashes and restarts
- The queue does not detect worker failure, then the queue can't remove it until
  the queue sends it a request