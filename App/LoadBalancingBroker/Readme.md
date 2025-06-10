# Load-Balancing Broker

- Use REQ - ROUTER combination
- Clients use REQ sockets
- Workers use REQ sockets
- Advantage: workers can talk freely with clients
- Load balancer uses LRU algorithm

![Load Balancing Broker](https://raw.githubusercontent.com/da0p/GithubPage/main/docs/assets/load_balancing_broker.drawio.png)