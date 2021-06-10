#include "p2p_go_socket_utils.h"

#include <cstddef>
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>

/**
 * A namespaces which defines manipulatable client information collected throughout
 * the runtime of the program.
 */
namespace ClientInformation {
    std::string client_ip = "";
}

/******************************** STATIC FUNCTION DECLARATIONS ********************************/

static bool udp_fill_addr_info(struct addrinfo** addr_info, const char* port);

static bool tcp_fill_addr_info(struct addrinfo** addr_info, const char* port);

static void fill_hints_structure(struct addrinfo* hints, int family, int socktype, int protocol);

/******************************** HELPER FUNCTIONS ********************************/

/**
 * Opens a socket given the parameterized addrinfo.
 *
 * This addrinfo structure should be initialized as empty via memset() before using this function.
 *
 * If opening a udp socket, set parameter udp as true. For TCP, set value as false.
 *
 * Return value is identical to the return value of socket(), the socket file descriptor, or -1 on error.
 */
int open_socket(struct addrinfo** addr_info, const char* port, bool udp) {
    struct addrinfo *rp;
    int sock_fd = -1;
    bool(*address_function)(struct addrinfo**, const char*) = udp ? *udp_fill_addr_info : *tcp_fill_addr_info;
    if (address_function(addr_info, port)) {
        int i = 0;
        for (rp = *addr_info; rp != nullptr; rp = rp->ai_next) {
            i++;
        }
        std::cout << "Total addr_info structures returned = " << i << std::endl;
        std::cout << "Opening socket..." << std::endl;
        sock_fd = socket((*addr_info)->ai_family, (*addr_info)->ai_socktype, (*addr_info)->ai_protocol);
    }
    return sock_fd;
}

/**
 * Sets the timeout of a given socket. Returns true if timeout was set, false otherwise.
 */
 bool set_socket_timeout(int sock_fd, int seconds) {
    if (seconds > 0) {
        struct timeval tv;
        tv.tv_sec = seconds;
        tv.tv_usec = 0;
        if (setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*) &tv, sizeof tv) == 0) {
            return true;
        } else {
            std::cout << "Error setting socket timeout, reason =" << strerror(errno) << std::endl;
            return false;
        }
    }
    std::cout << "Cannot set timeout less than or equal to zero." << std::endl;
    return false;
}

/**
 * Creates a confirm message to send to our client indicating that we have stored their IP address.
 * This message will look like:
 * "NLOS_CONNECTION_CONFIRM xxx.xxx.xxx.xxx"
 * Where xxx.xxx.xxx.xxx is their IP address we have stored.
 * Client should take note of the IP we have stored in the case they are using a wildcard IP to open their original socket.
 *
 * If using a Java BufferedReader to recieve this message
 * pass true in for add_newline_char to ensure it notes the end of the line!
 */
std::string get_confirm_message(std::string ip_address, bool add_newline_char) {
    std::string msg = CONFIRM_CONNECTION_MSG + " " + ip_address;
    if (add_newline_char)
        return msg + "\n";
    else
        return msg;
}

/**
 * Fetches the IP from a sockaddr_in structure.
 * Returns the IP as a std::string.
 */
std::string get_sockaddr_ip(struct sockaddr_in* client_info) {
    char client_ip_buffer[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_info->sin_addr), client_ip_buffer, INET_ADDRSTRLEN);
    return std::string(client_ip_buffer);
}


/******************************** STATIC HELPER FUNCTIONS ********************************/

/**
 * Fills a addrinfo structure with the values indicating that
 * a socket opened with the values in the parameterized structure will be a UDP server on the given port.
 * Returns true based on the address information being properly filled via "getaddrinfo()"
 */
static bool udp_fill_addr_info(struct addrinfo** addr_info, const char* port) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    fill_hints_structure(&hints, AF_INET, SOCK_DGRAM, 0);
    ///Fill address info structure with getaddrinfo.
    int addr_filled = getaddrinfo(OUR_IP.c_str(), port, &hints, addr_info);
    return addr_filled == 0;
}

/**
 * Fills a addrinfo structure with the values indicating that
 * a socket opened with the values in the parameterized structure will be a TCP server on the given port.
 * Returns true based on the address information being properly filled via "getaddrinfo()"
 */
static bool tcp_fill_addr_info(struct addrinfo** addr_info, const char* port) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    fill_hints_structure(&hints, AF_INET, SOCK_STREAM, 0);
    ///Fill address info structure with getaddrinfo.
    int addr_filled = getaddrinfo(OUR_IP.c_str(), port, &hints, addr_info);
    return addr_filled == 0;
}

/**
 * Fills an addrinfo hints structure with their family type, socket type, and protocol type.
 * If no specific protocol is to be set, pass parameter the integer 0.
 */
static void fill_hints_structure(struct addrinfo* hints, int family, int socktype, int protocol) {
    hints->ai_family = family;
    hints->ai_socktype = socktype;
    hints->ai_protocol = protocol;
    hints->ai_canonname = nullptr;
    hints->ai_addr = nullptr;
    hints->ai_next = nullptr;
}
