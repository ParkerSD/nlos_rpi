#ifndef P2P_GO_SOCKET_HEADER
#define P2P_GO_SOCKET_HEADER
#include <stddef.h>
#include <netinet/in.h>
#include <string>

/**
 * Listens for a UDP (Datagram) message from our connected p2p client.
 *
 * Will block until receiving a message, upon recieving this message, a response is sent to the client.
 * This response consists of the message "NLOS_CONNECTION_CONFIRM" followed by the IP address we have taken note of, forming the string:
 *
 * "NLOS_CONNECTION_CONFIRM xxx.xxx.xxx.xxx".
 *
 * If the client is using a wildcard IP to send out their initial message to us, it is their responsability to store the IP
 * this function sends in the confirmation message to ensure they recieve future messages from us.
 */
bool udp_connection_handshake();

/**
 * Opens and binds a udp socket.
 * Upon success, returns a socket file descriptor which can be used to recieve and send UDP packets.
 * On error, returns -1 (SOCKET_ERR)
 */
int udp_open_and_bind_socket();

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

/**
 * Fills a addrinfo structure with the values indicating that
 * a socket opened with the values in the para structure will be a UDP server.
 * Returns true based on the address information being properly filled via "getaddrinfo()"
 */
bool udp_fill_addr_info(struct addrinfo** addr_info);

/**
 * Fills a addrinfo structure with the values indicating that
 * a socket opened with the values in the parameterized structure will be a TCP server.
 * Returns true based on the address information being properly filled via "getaddrinfo()"
 */
bool tcp_fill_addr_info(struct addrinfo** addr_info);

#endif
