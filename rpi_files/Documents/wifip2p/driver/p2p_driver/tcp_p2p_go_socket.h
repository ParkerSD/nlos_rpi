#ifndef TCP_P2P_GO_SOCKET_HEADER
#define TCP_P2P_GO_SOCKET_HEADER
#include <string>
#include <sys/socket.h>

/**
 * Performs all neccessary operations to initialize a TCP socket:
 * - Opens socket.
 * - Binds socket.
 * - Listens on socket.
 * Upon success, returns a socket file descriptor which can be used to recieve and send TCP packets.
 * On error, returns -1 (SOCKET_ERR).
 */
int tcp_init_socket(const char* port);

/**
 * Sends a string over an open socket.
 * This uses TCP to send the string, so it WILL block until the message is sent!
 * Returns how many bytes were sent. Returns -1 if error sending bytes.
 */
int tcp_send_string(int sock_fd, std::string message, int flags);

/**
 * Opens and accepts a TCP connection. This performs both the opening of an original socket,
 * and the accepting of the connection to this socket.
 * BLOCKS until socket is opened and accepted!
 * orig_sock_fd should be memory ready to store a socket file descriptor. Will be overwritten with the originally opened socket fd.
 * port is the port to open the socket on.
 * client_addr needs to be pre allocated memory using memset().
 * Returns the socket fd of the newly opened socket address.
 */
int tcp_init_accept_socket(int* orig_sock_fd, const char* port, struct sockaddr_in* client_addr, socklen_t client_size);

#endif //TCP_P2P_GO_SOCKET_HEADER
