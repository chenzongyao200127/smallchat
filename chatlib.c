#define _POSIX_C_SOURCE 200112L
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ======================== Low level networking stuff ==========================
 * Here you will find basic socket stuff that should be part of
 * a decent standard C library, but you know... there are other
 * crazy goals for the future of C: like to make the whole language an
 * Undefined Behavior.
 * =========================================================================== */

/* Set the specified socket in non-blocking mode, with no delay flag. */
int socketSetNonBlockNoDelay(int fd)
{
    int flags, yes = 1;

    /* Set the socket nonblocking.
     * Note that fcntl(2) for F_GETFL and F_SETFL can't be
     * interrupted by a signal. */
    if ((flags = fcntl(fd, F_GETFL)) == -1)
        return -1;
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        return -1;

    /* This is best-effort. No need to check for errors. */
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes));
    return 0;
}

/* Create a TCP socket listening to 'port' ready to accept connections. */
int createTCPServer(int port)
{
    int s, yes = 1;
    struct sockaddr_in sa;

    // Create a socket using the socket() system call.
    // AF_INET specifies the address family as IPv4.
    // SOCK_STREAM denotes that the socket is of type TCP.
    // The third argument 0 allows the operating system to choose the appropriate protocol (TCP for stream sockets).
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return -1; // If socket creation fails, return -1.

    // Set socket options using setsockopt().
    // This line sets the SO_REUSEADDR option, allowing the socket to be quickly reused after the server is restarted.
    // The 'yes' variable is used to enable this option.
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)); // Best effort, not critical if it fails.

    // Initialize the sockaddr_in structure to zero using memset().
    // This structure holds information about the IP address and port number for the socket.
    memset(&sa, 0, sizeof(sa));

    // Set the address family to AF_INET (IPv4).
    sa.sin_family = AF_INET;

    // Convert the port number from host byte order to network byte order using htons().
    sa.sin_port = htons(port);

    // Set the IP address to INADDR_ANY, allowing the server to bind to all available interfaces.
    // htonl() converts the address to network byte order.
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind the socket to the specified port and address using bind().
    // If binding fails, or if listening on the socket fails, clean up by closing the socket and return -1.
    if (bind(s, (struct sockaddr *)&sa, sizeof(sa)) == -1 ||
        listen(s, 511) == -1) // The number 511 is the maximum number of pending connections in the queue.
    {
        close(s);  // Close the socket if either bind or listen fails.
        return -1; // Return -1 indicating failure.
    }

    // If everything is successful, return the socket descriptor.
    return s;
}

/* Create a TCP socket and connect it to the specified address.
 * On success the socket descriptor is returned, otherwise -1.
 *
 * If 'nonblock' is non-zero, the socket is put in nonblocking state
 * and the connect() attempt will not block as well, but the socket
 * may not be immediately ready for writing. */
// Attempts to create a TCP connection to the specified address and port.
// 'nonblock' determines if the socket should be set to non-blocking mode.
int TCPConnect(char *addr, int port, int nonblock)
{
    int s, retval = -1;
    struct addrinfo hints, *servinfo, *p;

    // String to hold the port number. Max length for a 16-bit number is 5 characters plus null terminator.
    char portstr[6];
    snprintf(portstr, sizeof(portstr), "%d", port);

    // Initialize hints to zero and set the address family and socket type.
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;     // Allow IPv4 or IPv6.
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets.

    // Resolve the server address and port.
    // If getaddrinfo fails, return -1.
    if (getaddrinfo(addr, portstr, &hints, &servinfo) != 0)
        return -1;

    // Loop through all the results and attempt to connect to each until successful.
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        /* Try to create the socket and to connect it.
         * If we fail in the socket() call, or on connect(), we retry with
         * the next entry in servinfo. */
        if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        // If non-blocking mode is requested, set the socket accordingly.
        if (nonblock && socketSetNonBlockNoDelay(s) == -1)
        {
            close(s); // Close the socket if setting non-blocking mode fails.
            break;
        }

        // Attempt to connect to the server.
        if (connect(s, p->ai_addr, p->ai_addrlen) == -1)
        {
            // Non-blocking mode: it's normal for connect() to return EINPROGRESS.
            if (errno == EINPROGRESS && nonblock)
                return s;

            // Other errors: close the socket and break from the loop.
            close(s);
            break;
        }

        // Successful connection.
        retval = s;
        break;
    }

    // Free the linked-list of addrinfo structures.
    freeaddrinfo(servinfo);

    // Return the socket descriptor, or -1 if no connection succeeded.
    return retval;
}

/* If the listening socket signaled there is a new connection ready to
 * be accepted, we accept(2) it and return -1 on error or the new client
 * socket on success. */
// This function takes a server socket file descriptor and waits for a client connection.
int acceptClient(int server_socket)
{
    int s; // Variable to store the client socket descriptor.

    while (1)
    {
        // Declare a sockaddr_in structure to store client information.
        struct sockaddr_in sa;
        // socklen_t is used to specify the size of the structure.
        socklen_t slen = sizeof(sa);

        // The accept() system call waits for an incoming client connection.
        // server_socket is the listening socket.
        // (struct sockaddr*)&sa is a pointer to a structure where the connecting client's address will be stored.
        // &slen is a pointer to a variable that stores the size of the client address structure.
        s = accept(server_socket, (struct sockaddr *)&sa, &slen);

        // If accept() returns -1, an error occurred.
        if (s == -1)
        {
            // If the error is EINTR (interrupted system call), the loop continues to retry the accept call.
            // This can happen if a signal is caught during the accept call.
            if (errno == EINTR)
                continue; /* Try again. */

            // For other errors, the function returns -1 indicating an error.
            else
                return -1;
        }
        // If accept() is successful, break out of the loop.
        break;
    }
    // Return the client socket descriptor.
    return s;
}

/* We also define an allocator that always crashes on out of memory: you
 * will discover that in most programs designed to run for a long time, that
 * are not libraries, trying to recover from out of memory is often futile
 * and at the same time makes the whole program terrible. */
// A custom malloc function for allocating memory.
void *chatMalloc(size_t size)
{
    // Attempt to allocate memory of the specified size.
    void *ptr = malloc(size);

    // Check if the memory allocation failed.
    if (ptr == NULL)
    {
        // If malloc returns NULL, it indicates memory allocation failure.
        // perror prints an error message to stderr, describing why malloc failed (typically "Out of memory").
        perror("Out of memory");

        // exit(1) terminates the program with a non-zero status, indicating an error.
        // This is a hard exit, meaning that the program will stop immediately without cleaning up.
        exit(1);
    }

    // Return the pointer to the allocated memory.
    return ptr;
}

/* Also aborting realloc(). */
// A custom realloc function for resizing allocated memory.
void *chatRealloc(void *ptr, size_t size)
{
    // Attempt to resize the memory block pointed to by ptr to the new size.
    ptr = realloc(ptr, size);

    // Check if the reallocation failed.
    if (ptr == NULL)
    {
        // If realloc returns NULL, it indicates memory reallocation failure.
        // perror prints an error message to stderr, similar to chatMalloc.
        perror("Out of memory");

        // Exit the program with an error status.
        exit(1);
    }

    // Return the pointer to the reallocated (or newly allocated) memory.
    return ptr;
}
