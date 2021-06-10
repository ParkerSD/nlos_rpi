#ifndef P2P_GO_SOCKET_UTILS_HEADER
#define P2P_GO_SOCKET_UTILS_HEADER

#include <netdb.h>
#include <string>

/**
 * A string representing the port we will be conducting general communication
 * on from both this driver and the app.
 */
#define GENERAL_PORT_STR "8988"

/**
 * A string representing the port we will be listening on for control commands.
 */
#define COMMAND_PORT_STR "8488"

/** The maximum amount of bytes we allow to be received in one message. */
#define MAX_RECV 1024

/** A macro indicating that a socket error has occurred within our program. */
#define NLOS_SOCKET_ERR -1

/** A macro indicating that there has been an error receiving a message. */
#define NLOS_RECV_ERR -2

/** A macro defining the maximum amount of TCP connect() requests we allow to be stored on the backlog. */
#define BACKLOG 2

/******************************** CONSTANT STRINGS ********************************/

/** Our static IP address which should have been configured with our systemd-networkd setup process. */
const std::string OUR_IP = "192.168.4.1";

/** A prefix to the message we send out indicating that we have stored their IP address. */
const std::string CONFIRM_CONNECTION_MSG = "NLOS_CONNECTION_CONFIRM";


/******************************** CLIENT NAMESPACE ********************************/

/**
 * A namespace which defines manipulatable client information collected throughout
 * the runtime of the program.
 */
namespace ClientInformation {
    extern std::string client_ip;
}

/******************************** FUNCTION DECLARATIONS ********************************/

/**
 * Opens a socket given the parameterized addrinfo.
 *
 * This addrinfo structure should be initialized as empty via memset() before using this function.
 *
 * If opening a udp socket, set parameter udp as true. For TCP, set value as false.
 *
 * Return value is identical to the return value of socket(), the socket file descriptor, or -1 on error.
 */
int open_socket(struct addrinfo** addr_info, const char* port, bool udp);

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
std::string get_confirm_message(std::string ip_address, bool add_newline_char);

/**
 * Fetches the IP from a sockaddr_in structure.
 * Returns the IP as a std::string.
 */
std::string get_sockaddr_ip(struct sockaddr_in* client_info);

/**
 * Sets the timeout of a given socket. Returns true if timeout was set, false otherwise.
 */
 bool set_socket_timeout(int sock_fd, int seconds);

#endif // P2P_GO_SOCKET_UTILS_HEADER
