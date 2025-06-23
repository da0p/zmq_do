# Majordomo Pattern

Majordomo pattern is similar to Paranoid Pirate pattern in ensuring reliability

- between client and broker using retry mechanism
- between broker and workers using two-way heartbeats

However, Majordomo's architecture is more service-oriented. Each worker will
register its provided service when connecting to the broker