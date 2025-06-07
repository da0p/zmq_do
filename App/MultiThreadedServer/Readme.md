# Rules for Multithreaded Code with ZeroMQ

- Isolate data privately within its thread and never share data in multiple
  threads. The only exception to this are ZeroMQ contexts, which are threadsafe
- Stay away from the classic concurrency mechanisms like as mutexes, critical
  sections, semaphores, etc. These are an anti-pattern in ZeroMQ applications
- Create one ZeroMQ context at the start of your process, and pass that to all
  threads that you want to connect via _inproc_ sockets
- Use _attached_ threads to create structure within your application, and
  connect these to their parent threads using _PAIR_ sockets over _inproc_. The
  pattern is: bind parent socket, then create child thread which connects its
  socket
- Use _detached_ threads to simulate independent tasks, with their own contexts.
  Connect these over _tcp_. Later these can be moved stand-alone processes
  without changing the code significantly
- All interaction between threads happens as ZeroMQ messages, which you can
  define more or less formally
- Don't share ZeroMQ sockets between threads. ZeroMQ sockets are not threadsafe

# MultiThreaded Server

Using inproc zmq socket to communicate between parent thread and child threads

![Multithreaded Server](https://raw.githubusercontent.com/da0p/GithubPage/main/docs/assets/multithreaded_server.drawio.png)