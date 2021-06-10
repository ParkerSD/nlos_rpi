#include "p2p_go_socket_utils.h"
#include "tcp_p2p_go_socket.h"
#include "command_handler.h"
#include "command_executor.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <cstddef>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <iostream>
#include <stdio.h>
#include <string>

/** The maximum bytes we will be storing as a buffer for commands. */
#define COMMAND_MAX_RECV 64

/** Seconds our socket will timeout on while waiting for commands. This is to update the connected value. */
#define COMMAND_SOCKET_TIMEOUT 4

/******************************** STATIC FUNCTION DECLARATIONS ********************************/

static void command_recv_timeout(int sock_fd, fd_set* in_set);

static void execute_command(char* command_buffer, int bytes_received);


/******************************** CONTROL OPTIONS NAMESPACE ********************************/

/**
 * Control option characters used to specify commands sent to this device.
 * See NLOS Control Packet Specification document for in depth documentation.
 */
namespace ControlOptions {
    const char control_byte = 'M';
    const char flashlight = 'F';
    const char left_led = 'L';
    const char right_led = 'R';
    const char ready = 'B';
    const char stop = 'S';
    const char terminate = 'T';
    const char on = 'O';
    const char off = 'X';
    const char no_option = 'N';
}

/******************************** GLOBAL VARIABLES FOR MANIPULATION ********************************/

/**
 * Indicates if we should be running our command loop.
 * Should ONLY be altered and accessed by the functions:
 * - set_listen_for_commands()
 * - is_listening_for_commands()
 */
static bool run_command_loop = false;

/**
 * Mutex for locking access to the run_command_loop variable.
 */
static std::mutex run_mutex;

/******************************** COMMAND HANDLING FUNCTIONS ********************************/

/**
 * Opens a TCP socket on the port specified in the macro COMMAND_PORT_STR (p2p_go_socket_utils.h)
 * Listens for commands from the connected client, which are then parsed and used to control the NLOS hardware.
 * This function blocks, and is designed to run on its own thread.
 * Times out every COMMAND_SOCKET_TIMEOUT (n seconds) to check the current p2p connection status, indicated by run_command_loop.
 * If run_command_loop is false, this function will finish properly.
 */
void listen_for_commands() {
    ///Allocate memory for client info.
    struct sockaddr_in tcp_client_addr;
    memset(&tcp_client_addr, 0, sizeof tcp_client_addr);
    int sock_fd = -1;
    ///Open socket.
    int new_fd = tcp_init_accept_socket(&sock_fd, COMMAND_PORT_STR,
                    &tcp_client_addr, sizeof tcp_client_addr);
    if (new_fd == NLOS_SOCKET_ERR) {
        set_listen_for_commands(false);
        return;
    }
    std::cout << "Command socket initialized." << std::endl;
    fd_set in_set;
    bool run_placeholder = is_listening_for_commands();
    while (run_placeholder) {
        ///Buffer for receiving commands.
        char buffer[COMMAND_MAX_RECV];
        ///Use the select method to wait for messages.
        command_recv_timeout(new_fd, &in_set);
        ///If there is a message waiting for us, receive it.
        if (FD_ISSET(new_fd, &in_set)) {
            int bytes_received = recv(new_fd, buffer, COMMAND_MAX_RECV, 0);
            if (bytes_received > 0) {
               buffer[bytes_received] = '\0';
               execute_command(buffer, bytes_received);
            }
        }
        ///Update loop variable.
        run_placeholder = is_listening_for_commands();
    }
    std::cout << "Finished listening for commands." << std::endl;
    close(new_fd);
    close(sock_fd);
}

/**
 * Executes a command by calling the corresponding function in command_executor.h
 * Requires a buffer containing the command, alongisde the count of bytes received.
 * Only executes commands that have more than two bytes/chars.
 */
static void execute_command(char* command_buffer, int bytes_received) {
    ///Check for a properly formed command.
    if (bytes_received > 2 && command_buffer[0] == ControlOptions::control_byte) {
        char command = command_buffer[1];
        bool toggle_val = (command_buffer[2] == ControlOptions::on);
        switch(command) {
            case ControlOptions::flashlight:
                flashlight_toggle(toggle_val);
                break;
            case ControlOptions::left_led:
                left_led_toggle(toggle_val);
                break;
            case ControlOptions::right_led:
                right_led_toggle(toggle_val);
                break;
            case ControlOptions::ready:
                run_stream();
                break;
            case ControlOptions::stop:
                stop_stream();
                break;
            case ControlOptions::terminate:
                terminate_execution();
                break;
        }
    }
}

/**
 * Utilizes a lock guard to set whether we should continue to listen for commands.
 */
void set_listen_for_commands(bool val) {
    std::lock_guard<std::mutex> guard(run_mutex);
    run_command_loop = val;
}

/**
 * Utilizes a lock guard to return whether we should continue to listen for commands.
 */
bool is_listening_for_commands() {
    std::lock_guard<std::mutex> guard(run_mutex);
    return run_command_loop;
}

/**
 * Blocks for n seconds using select to wait for something ready to be received (using recv())
 * on the corresponding socket file descriptor.
 */
static void command_recv_timeout(int sock_fd, fd_set* in_set) {
    struct timeval timeout;
    timeout.tv_sec = COMMAND_SOCKET_TIMEOUT;
    timeout.tv_usec = 0;
    FD_ZERO(in_set);
    FD_SET(sock_fd, in_set);
    select(sock_fd + 1, in_set, nullptr, nullptr, &timeout);
}
