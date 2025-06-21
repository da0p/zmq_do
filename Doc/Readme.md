# Useful Notes

At first, I thought it's only for zmq, however, I can take some insights for
designing distributed systems in general

## Understanding Request Reply Sockets

- The **REQ** socket sends, to the network, an empty delimiter frame in front of the
  message data. REQ sockets are synchronous. REQ sockets always send one request
  and then wait for one reply. REQ sockets talk to one peer at a time. If you
  connect a REQ socket to multiple peers, requests are distributed to and replies
  expected from each peer one turn at a time
- The **REP** socket reads and saves all identity frames up to and including the
  empty delimiter, then passes the following frame or frames to the caller. REP
  sockets are synchronous and talk to one peer at a time. If you connect a REP
  socket to multiple peers, requests are read from peers in fair fashion, and
  replies are always sent to the same peer that made the last request
- The **DEALER** socket is oblivious to the reply envelope and handles this like
  any multipart message. DEALER sockets are asynchronous and like PUSH and PULL
  combined. They distribute sent messages among all connections, and fair-queue
  received messages from all connections
- The **ROUTER** socket is oblivious to the reply envelope, like DEALER. It creates
  identities for its connections, and passes these identities to the caller as a
  first frame in any received message. Conversely, when the caller sends a message,
  it uses the first message frame as an identity to look up the connection to
  send to. ROUTERS are asynchronous

![ZMQ Request Reply Sockets](https://raw.githubusercontent.com/da0p/GithubPage/main/docs/assets/zmq_req_rep_socket.drawio.png)

**DEALER** is like an asynchronous **REQ** socket, and **ROUTER** is like an
asynchronous **REP** socket. Where we use a REQ socket, we can use a DEALER; we
just have to read and write the envelope ourselves. Where we use a REP socket,
we can stick a ROUTER; we just need to manage identities ourselves

## Understanding Router Sockets

- **Identity** concept in ZeroMQ refers specifically to **ROUTER** sockets. The
  sockets use it to identify the connection to other sockets. If at the connection
  time, the peer doesn't say its identity, then **ROUTER** sockets will generate
  an arbitrary random identity for the connection (5 bytes). Note that the peer
  can set its identity with more than 5 bytes
- **ROUTER** sockets do not have a way of dealing with messages they can't send
  anywhere, so they drop them silently. If we don't want this feature, we can
  set _ZMQ\_ROUTER\_MANDATORY_ option, so that when there an unroutable identity
  on a send call, the socket will signal an _EHOSTUNREACH_ error

## Understanding Dealer Sockets

- **DEALER** socket does not send an empty delimiter frame before any data frames
  as the **REQ** socket 
- **DEALER** socket is fully asynchronous, it can send multiple request before
  receiving a reply, not like **REQ** socket

## Reliable Request-Reply Pattern

There are several request-reply patterns, all of them require require the usage
of heartbeat. The reason for it is due ot the fact that TCP has a long timeout
(30 minutes or so), and it is impossible to know whether a peer has died or been
disconnected.

### No Heartbeat Approach

- When a **ROUTER** socket is used in an application to track peers, as peers
  disconnect and reconnect, the application will leak memory and get slower
- When a **SUB** or **DEALER** is used, there is no way to tell whether the
  peer died
- If we use a TCP connection that stays silent for a while, it will, in some
  networks, just die. Sending something will keep the network alive

### One-way Heartbeats

- Food for PUB-SUB, only PUB should send heartbeats to SUB
- Can be inaccurate when a large amount of data is sent, as heartbeats will be
  delayed behind that data. Better to treat data also as a heartbeat
- PUB-SUB will drop messages for disappeared recipients, however, PUSH and DEALER
  will queue them. So if you send heartbeats to a dead peer and it comes back, it
  will get all the heartbeats you sent!

### Ping-Pong Heartbeats

- One peer seens a ping command to the other, which replies with a pong command.
  Neither command has any payload. Usually the client pings the server

## Idempotent Services 

Idempotent means that it is safe to repeat an operation. Not all operations are
idempotent

Examples of idempotent use cases

- Stateless task distribution, i.e., a pipeline where the servers are stateless
  workers that compute a reply based purely on the state provided by a request.
  In such a case, it's safe to execute the same request many times
- A name service that translates logical addresses into endpoints to bind or
  connect to 

Examples of non-idempotent use cases

- A logging service. One does not want the same log information recorded more
  than once
- Any service that has impact on downstream nodes, e.g., sends on information to
  other nodes. If that service gets the same request more than once, downstream
  nodes will get duplicate information
- Any service that modifies shared data in some non-idempotent way, a service
  that debits a bank account is not idempotent without extra work

When our server applications are not idempotent, it's essential to think more
carefully about when exactly they might crash!

In order to handle non-idempotent operations, use the fairly standard solution
of detecting and rejecting duplicate requests:

- The client must stamp every request with a unique client identifier and a unique
  message number
- The server, before sending back a reply, stores it using the combination of
  client ID and message number as a key
- The server, when getting a request from a given client, first checks whether
  it has a reply for that client ID and message number. If so, it does not process
  the request, but just resends the reply