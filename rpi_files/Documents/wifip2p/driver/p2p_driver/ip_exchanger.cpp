#include "p2p_go_socket_utils.h"
#include "tcp_p2p_go_socket.h"
#include "udp_p2p_go_socket.h"

#include <uvgrtp/lib.hh>
#include <cstddef>
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <errno.h>

/** The maximum amount of confirmed connection messages to send. */
const int MAX_CONFIRM_MSGS = 2;

/**
 * Listens for a UDP (Datagram) packet from our connected p2p client
 * on the port described by GENERAL_PORT_STR in p2p_go_sock_utils.h
 *
 * Will block until receiving a message, upon recieving this message, a response is sent to the client.
 * This response consists of the message "NLOS_CONNECTION_CONFIRM" followed by the IP address we have taken note of, forming the string:
 *
 * "NLOS_CONNECTION_CONFIRM xxx.xxx.xxx.xxx".
 *
 * If the client is using a wildcard IP to send out their initial message to us, it is their responsability to store the IP
 * this function sends in the confirmation message to ensure they recieve future messages from us.
 */
bool udp_connection_handshake() {
    ///Socket address for communicating with client.
    struct sockaddr_in udp_client_addr;

    ///Open socket.
    int sock_fd = udp_init_socket(GENERAL_PORT_STR);
    if (sock_fd < 0)
        return false;

    ///Allocate memory for client info.
    memset(&udp_client_addr, 0, sizeof udp_client_addr);
    size_t client_size = sizeof udp_client_addr;

    ///Buffer for receiving messages.
    char buffer[MAX_RECV];
    if (udp_receive_msg(sock_fd, buffer, (struct sockaddr*) &udp_client_addr, &client_size) < 0) {
        ///If we didn't receive a message or error occurred, clean up client addr structure and return false.
        memset(&udp_client_addr, 0, sizeof udp_client_addr);
        close(sock_fd);
        return false;
    }

    ///Fetch client ip & print recieved information.
    ClientInformation::client_ip = get_sockaddr_ip(&udp_client_addr);
    std::string confirm_message = get_confirm_message(ClientInformation::client_ip, false);
    std::cout << "Client Message: " << buffer << std::endl;
    std::cout << "Client information: " << std::endl;
    std::cout << "Client Addr = " << ClientInformation::client_ip << std::endl;

    ///Send out confirm messages.
    for (int i = 0; i < MAX_CONFIRM_MSGS; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        udp_send_string(sock_fd, &udp_client_addr, confirm_message, MSG_CONFIRM);
    }
    std::cout << "Sent confirmation messages." << std::endl;
    ///Close the socket. We are clear to start sending new data to the client's IP.
    ///It is up to the client to break the connection if they never receive these messages.
    close(sock_fd);
    return true;
}

/**
 * Listens for a TCP packet from our connected p2p client
 * on the port described by GENERAL_PORT_STR in p2p_go_sock_utils.h
 *
 * Will block until receiving a message, upon recieving this message, a response is sent to the client.
 * This response consists of the message "NLOS_CONNECTION_CONFIRM" followed by the IP address we have taken note of, forming the string:
 *
 * "NLOS_CONNECTION_CONFIRM xxx.xxx.xxx.xxx".
 *
 * If the client is using a wildcard IP to send out their initial message to us, it is their responsability to store the IP
 * this function sends in the confirmation message to ensure they recieve future messages from us.
 */
bool tcp_connection_handshake() {
    ///Socket address for communicating with client.
    struct sockaddr_in tcp_client_addr;

    ///Allocate memory for client info.
    memset(&tcp_client_addr, 0, sizeof tcp_client_addr);
    int sock_fd = -1;

    ///Open socket.
    int new_fd = tcp_init_accept_socket(&sock_fd, GENERAL_PORT_STR, &tcp_client_addr, sizeof tcp_client_addr);
    std::cout << "Connection accept completed." << std::endl;
    ClientInformation::client_ip = get_sockaddr_ip(&tcp_client_addr);
    std::cout << "Client IP: " << ClientInformation::client_ip << std::endl;

    char buffer[MAX_RECV];
    if (recv(new_fd, buffer, MAX_RECV, 0) < 0) {
        close(new_fd);
        close(sock_fd);
        return false;
    }
    ///Send a confirmation message.
    std::cout << "Client says " << buffer << std::endl;
    int bytes_sent = tcp_send_string(new_fd, get_confirm_message(ClientInformation::client_ip, true), 0);
    close(new_fd);
    close(sock_fd);
    return bytes_sent > 0;
}

