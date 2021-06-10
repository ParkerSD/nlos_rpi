#include "p2p_go_socket_utils.h"
#include "udp_p2p_go_socket.h"

#include <cstddef>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <errno.h>
#include <chrono>

/** The maximum amount of confirmed connection messages to send. */
const int MAX_CONFIRM_MSGS = 5;

/**
 * Opens and binds a udp socket.
 * Upon success, returns a socket file descriptor which can be used to recieve and send UDP packets.
 * On error, returns -1 (SOCKET_ERR)
 */
int udp_init_socket(const char* port) {
    ///Address info structure, used to open the socket.
    struct addrinfo *addr_info;
    memset(&addr_info, 0, sizeof addr_info);

    ///Open our UDP socket.
    int sock_fd = open_socket(&addr_info, port, true);

    ///If socket failed to be opened, return.
    if (sock_fd < 0) {
        std::cout << "Error opening server socket: " << strerror(errno) << std::endl;
        //freeaddrinfo(addr_info);
        return NLOS_SOCKET_ERR;
    }

    ///Bind socket so we can send/receive information.
    if (bind(sock_fd, addr_info->ai_addr, addr_info->ai_addrlen) < 0) {
        close(sock_fd);
        std::cout << "Error binding server socket: " << strerror(errno) << std::endl;
        //freeaddrinfo(addr_info);
        return NLOS_SOCKET_ERR;
    }

    std::cout << "Binded successfully" << std::endl;
    //freeaddrinfo(addr_info);
    return sock_fd;
}

/**
 * Sends a string to a client address using the function sendto().
 * sock_fd must be the file descriptor of an open socket.
 */
int udp_send_string(int sock_fd, struct sockaddr_in* udp_client_addr, std::string message, int msg_flags) {
    socklen_t client_len = (socklen_t) sizeof *udp_client_addr;
    int bytes_sent = sendto(sock_fd, message.c_str(),
                        message.length(), msg_flags,
                        (const struct sockaddr*) udp_client_addr, client_len);
    if (bytes_sent < 0)
        std::cout << "Error sending packets, reason = " << strerror(errno) << std::endl;
    else
        std::cout << "Sent " << bytes_sent << " bytes " << std::endl;
    return bytes_sent;
}


/**
 * Recieves a UDP (Datagram) packet on an opened socket.
 * Returns the length of the message & fills the parameterized char* buffer with the recieved message.
 * **WILL BLOCK UNTIL MESSAGE IS RECEIVED**
 */
int udp_receive_msg(int sock_fd, char* buffer, struct sockaddr* udp_client_addr, size_t* client_len) {
    std::cout << "Attempting to receive message ... " << std::endl;
    ///Try to receive a message from our client.
    int bytes_received = recvfrom(sock_fd, (char *) buffer, MAX_RECV, MSG_WAITALL, udp_client_addr, client_len);
    if (bytes_received < 0) {
        close(sock_fd);
        return NLOS_RECV_ERR;
    }
    std::cout << "Bytes receieved: " << std::endl;
    buffer[bytes_received] = '\0';
    return bytes_received;
}
