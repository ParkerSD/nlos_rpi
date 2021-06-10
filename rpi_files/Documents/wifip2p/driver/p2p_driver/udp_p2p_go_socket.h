#ifndef UDP_P2P_GO_SOCKET_HEADER
#define UDP_P2P_GO_SOCKET_HEADER

#include <stddef.h>
#include <netinet/in.h>
#include <string>

/**
 * Performs all neccessary operations to initialize a UDP socket:
 * - Opens socket.
 * - Binds socket.
 * Upon success, returns a socket file descriptor which can be used to recieve and send UDP packets.
 * On error, returns -1 (SOCKET_ERR).
 */
int udp_init_socket(const char* port);

/**
 * Recieves a UDP (Datagram) packet on an opened socket.
 * Returns the length of the message & fills the parameterized char* buffer with the recieved message.
 * **WILL BLOCK UNTIL MESSAGE IS RECEIVED**
 */
int udp_receive_msg(int sock_fd, char* buffer, struct sockaddr* udp_client_addr, size_t* client_len);

/**
 * Sends a string to a client address using the function sendto().
 * sock_fd must be the file descriptor of an open socket.
 */
int udp_send_string(int sock_fd, sockaddr_in* udp_client_addr, std::string message, int msg_flags);

#endif
