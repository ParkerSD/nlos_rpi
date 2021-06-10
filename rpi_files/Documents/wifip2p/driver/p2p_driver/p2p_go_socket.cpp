#include"p2p_go_socket.h"

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

/** A string representing the port we will be conducting communication on from both this driver and the app. */
#define PORT_STR "8988"

/** The maximum amount of bytes we allow to be received in one message. */
#define MAX_RECV 1024

/** A macro indicating that a socket error has occurred within our program. */
#define SOCKET_ERR -1

/** A macro indicating that there has been an error receiving a message. */
#define RECV_ERR -1

/******************************** STATIC FUNCTION DECLARATIONS ********************************/

static void fill_hints_structure(struct addrinfo* hints, int family, int socktype, int protocol);

/******************************** CONSTANT STRINGS ********************************/

/** A message to send to the connection device indicating that we confirm their IP being stored. */
const std::string CONFIRM_CONNECTION_MSG = "NLOS_CONNECTION_CONFIRM ";

/** Our static IP address which should have been configured with our systemd-networkd setup process. */
const std::string OUR_IP = "192.168.4.1";


/******************************** UDP SOCKET FUNCTIONS ********************************/


/** The maximum amount of confirmed connection messages to send. */
const int MAX_CONFIRM_MSGS = 5;

/** A socket address of the client which we will be communicating with. */
struct sockaddr_in udp_client_addr;

std::string their_ip = "";

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
bool udp_connection_handshake() {

    ///Open socket.
    int sock_fd = udp_open_and_bind_socket();
    if (sock_fd < 0)
        return false;

    ///Allocate memory for the peer info.
    memset(&udp_client_addr, 0, sizeof udp_client_addr);
    size_t client_len = sizeof udp_client_addr;

    ///Buffer for receiving messages.
    char buffer[MAX_RECV];
    if (udp_receive_msg(sock_fd, buffer, (struct sockaddr*) &udp_client_addr, &client_len) < 0) {
        ///If we didn't receive a message or error occurred, clean up client addr structure and return false.
        memset(&udp_client_addr, 0, sizeof udp_client_addr);
        close(sock_fd);
        return false;
    }

    ///Fetch client ip & print recieved information.
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &((&udp_client_addr)->sin_addr), client_ip, INET_ADDRSTRLEN);
    std::cout << "Client Message: " << buffer << std::endl;
    std::cout << "Client information: " << std::endl;
    std::cout << "Client Addr = " << client_ip << std::endl;

    ///Build client message and send out confirm messages.
    std::string confirm_message = CONFIRM_CONNECTION_MSG + " " + client_ip;
    for (int i = 0; i < MAX_CONFIRM_MSGS; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        udp_send_string(sock_fd, &udp_client_addr, confirm_message, MSG_CONFIRM);
    }
    std::cout << "Sent confirmation messages." << std::endl;
    their_ip = client_ip;
    ///Close the socket. We are clear to start sending new data to the client's IP.
    ///It is up to the client to break the connection if they never receive these messages.
    close(sock_fd);
    return true;
}

/**
 * Opens and binds a udp socket.
 * Upon success, returns a socket file descriptor which can be used to recieve and send UDP packets.
 * On error, returns -1 (SOCKET_ERR)
 */
int udp_open_and_bind_socket() {
    ///Address info structure, used to open the socket.
    struct addrinfo *addr_info;
    memset(&addr_info, 0, sizeof addr_info);
    ///Fill the udp server hints of our socket.
    int sock_fd = open_socket(addr_info, *udp_fill_addr_info);

    ///If socket failed to be opened, return.
    if (sock_fd < 0) {
        std::cout << "Error opening server socket: " << strerror(errno) << std::endl;
        freeaddrinfo(addr_info);
        return SOCKET_ERR;
    }

    ///Bind socket so we can send/receive information.
    if (bind(sock_fd, addr_info->ai_addr, addr_info->ai_addrlen) < 0) {
        close(sock_fd);
        std::cout << "Error binding server socket: " << strerror(errno) << std::endl;
        freeaddrinfo(addr_info);
        return SOCKET_ERR;
    }
    std::cout << "Binded successfully" << std::endl;
    freeaddrinfo(addr_info);
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
        return RECV_ERR;
    }
    buffer[bytes_received] = '\0';
    return bytes_received;
}

/**
 * Fills a addrinfo structure with the values indicating that
 * a socket opened with the values in the parameterized structure will be a UDP server.
 * Returns true based on the address information being properly filled via "getaddrinfo()"
 */
bool udp_fill_addr_info(struct addrinfo** addr_info) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    fill_hints_structure(&hints, AF_INET, SOCK_DGRAM, 0);
    ///Fill address info structure with getaddrinfo.
    int addr_filled = getaddrinfo(OUR_IP.c_str(), PORT_STR, &hints, addr_info);
    freeaddrinfo(&hints);
    return addr_filled == 0;
}

/******************************** TCP SOCKET FUNCTIONS ********************************/

/**
 * Opens and binds, and listens using a TCP socket.
 * On success, the TCP socket's file descriptor will be returned.
 * On failure, -1 will be returned.
 */
int tcp_open_and_bind_socket() {
    ///Address info structure, used to open the socket.
    struct addrinfo *addr_info;
    memset(&addr_info, 0, sizeof addr_info);
    ///Fill the udp server hints of our socket.
    int sock_fd = open_socket(addr_info, *tcp_fill_addr_info);
    ///If socket failed to be opened, return.
    if (sock_fd < 0) {
        std::cout << "Error opening server socket: " << strerror(errno) << std::endl;
        freeaddrinfo(addr_info);
        return SOCKET_ERR;
    }
    return SOCKET_ERR;
}

/**
 * Fills a addrinfo structure with the values indicating that
 * a socket opened with the values in the parameterized structure will be a TCP server.
 * Returns true based on the address information being properly filled via "getaddrinfo()"
 */
bool tcp_fill_addr_info(struct addrinfo** addr_info) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    fill_hints_structure(&hints, AF_INET, SOCK_STREAM, 0);
    ///Fill address info structure with getaddrinfo.
    int addr_filled = getaddrinfo(OUR_IP.c_str(), PORT_STR, &hints, addr_info);
    freeaddrinfo(&hints);
    return addr_filled == 0;
}

static void fill_hints_structure(struct addrinfo* hints, int family, int socktype, int protocol) {
    hints->ai_family = family;
    hints->ai_socktype = socktype;
    hints->ai_protocol = protocol;
    hints->ai_canonname = nullptr;
    hints->ai_addr = nullptr;
    hints->ai_next = nullptr;
}
