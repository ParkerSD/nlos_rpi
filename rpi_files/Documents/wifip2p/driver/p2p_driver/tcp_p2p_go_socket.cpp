#include "tcp_p2p_go_socket.h"
#include "p2p_go_socket_utils.h"

#include <uvgrtp/lib.hh>
#include <cstddef>
#include <netdb.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <errno.h>

/**
 * Sends a string over an open socket.
 * This uses TCP to send the string, so it WILL block until the message is sent!
 * Returns how many bytes were sent. Returns -1 if error sending bytes.
 */
int tcp_send_string(int sock_fd, std::string message, int flags) {
    int bytes_sent = send(sock_fd, message.c_str(), message.length(), 0);
    if (bytes_sent < 0)
        std::cout << "Error sending response: " << strerror(errno) << std::endl;
    else
        std::cout << "Sent " << bytes_sent << " bytes." << std::endl;
    return bytes_sent;
}

/**
 * Opens and binds, and listens using a TCP socket.
 * On success, the TCP socket's file descriptor will be returned.
 * This file descriptor can be used to recieve and send TCP packets.
 * On failure, -1 will be returned.
 */
int tcp_init_socket(const char* port) {
    ///Address info structure, used to open the socket.
    struct addrinfo *addr_info;
    memset(&addr_info, 0, sizeof addr_info);

    ///Fill the udp server hints of our socket.
    int sock_fd = open_socket(&addr_info, port, false);

    ///If socket failed to be opened, return.
    if (sock_fd < 0) {
        std::cout << "Error opening server socket: " << strerror(errno) << std::endl;
        return NLOS_SOCKET_ERR;
    }

    ///Bind socket so we can send/receive information.
    if (bind(sock_fd, addr_info->ai_addr, addr_info->ai_addrlen) < 0) {
        close(sock_fd);
        std::cout << "Error binding server socket: " << strerror(errno) << std::endl;
        return NLOS_SOCKET_ERR;
    }

    ///Listen on socket.
    if (listen(sock_fd, BACKLOG) < 0) {
        close(sock_fd);
        std::cout << "Error listening on socket: " << strerror(errno) <<std::endl;
        return NLOS_SOCKET_ERR;
    }

    ///Finished setting up TCP socket. Free addr info and return socket fd.
    freeaddrinfo(addr_info);
    return sock_fd;
}

/**
 * Opens and accepts a TCP connection. This performs both the opening of an original socket,
 * and the accepting of the connection to this socket.
 * BLOCKS until socket is opened and accepted!
 * orig_sock_fd should be memory ready to store a socket file descriptor. Will be overwritten with the originally opened socket fd.
 * port is the port to open the socket on.
 * client_addr needs to be pre allocated memory using memset().
 * Returns the socket fd of the newly opened socket address.
 */
int tcp_init_accept_socket(int* orig_sock_fd, const char* port, struct sockaddr_in* client_addr, socklen_t client_size) {
    *orig_sock_fd = tcp_init_socket(port);
    if (*orig_sock_fd == NLOS_SOCKET_ERR) ///Error opening original socket. Return socket error.
        return NLOS_SOCKET_ERR;
    ///Accept a connection from the client.
    int new_fd = accept(*orig_sock_fd, (struct sockaddr*) client_addr, &client_size);
    if (new_fd < 0) {
        std::cout << "Error accepting connect." << std::endl;
        return NLOS_SOCKET_ERR;
    }
    return new_fd;
}
