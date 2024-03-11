// Header guards to prevent multiple inclusion of this header file.
#ifndef CHATLIB_H
#define CHATLIB_H

/* Networking. */
// Function prototypes for networking-related functions.

// createTCPServer: Initializes a TCP server listening on the specified port.
int createTCPServer(int port);

// socketSetNonBlockNoDelay: Sets a socket to non-blocking mode and disables Nagle's algorithm.
int socketSetNonBlockNoDelay(int fd);

// acceptClient: Accepts a client connection on a given server socket.
int acceptClient(int server_socket);

// TCPConnect: Establishes a TCP connection to a specified address and port, with an option for non-blocking mode.
int TCPConnect(char *addr, int port, int nonblock);

/* Allocation. */
// Function prototypes for memory allocation-related functions.

// chatMalloc: A wrapper around malloc with error checking and program termination on failure.
void *chatMalloc(size_t size);

// chatRealloc: A wrapper around realloc, also with error checking and program termination on failure.
void *chatRealloc(void *ptr, size_t size);

// End of the header guard.
#endif // CHATLIB_H
