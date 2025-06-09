# Understanding Request Reply Sockets

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

# Understanding Router Dealer Sockets

- **Identity** concept in ZeroMQ refers specifically to **ROUTER** sockets. The
  sockets use it to identify the connection to other sockets. If at the connection
  time, the peer doesn't say its identity, then **ROUTER** sockets will generate
  an arbitrary random identity for the connection (5 bytes). Note that the peer
  can set its identity with more than 5 bytes
- **ROUTER** sockets do not have a way of dealing with messages they can't send
  anywhere, so they drop them silently. If we don't want this feature, we can
  set _ZMQ\_ROUTER\_MANDATORY_ option, so that when there an unroutable identity
  on a send call, the socket will signal an _EHOSTUNREACH_ error