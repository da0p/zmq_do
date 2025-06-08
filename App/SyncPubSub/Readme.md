# Synchronous PubSub

- The publisher knows how many subscribers it should expect before sending out messages
- Note that we need to set send high-water mark limit and do not allow to drop messages silently
- After the publisher receives all confirmations from subscribers, it starts to publish data

![Synchronous PubSub](https://raw.githubusercontent.com/da0p/GithubPage/main/docs/assets/sync_pubsub.drawio.png)