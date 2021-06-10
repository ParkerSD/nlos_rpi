#ifndef COMMAND_HANDLER_HEADER

#define COMMAND_HANDLER_HEADER

/**
 * Control option characters used to specify commands sent to this device.
 * See NLOS Control Packet Specification document for in depth documentation.
 */
namespace ControlOptions {
    extern const char control_byte;
    extern const char flashlight;
    extern const char left_led;
    extern const char right_led;
    extern const char ready;
    extern const char stop;
    extern const char terminate;
    extern const char on;
    extern const char off;
}

/**
 * Opens a TCP socket on the port specified in the macro COMMAND_PORT_STR (p2p_go_socket_utils.h)
 * Listens for commands from the connected client, which are then parsed and used to control the NLOS hardware.
 * This function blocks, and is designed to run on its own thread.
 * Times out every COMMAND_SOCKET_TIMEOUT (n seconds) to check the current p2p connection status, indicated by run_command_loop.
 * If run_command_loop is false, this function will finish properly.
 */
void listen_for_commands();

/**
 * Utilizes a lock guard to set whether we should continue to listen for commands.
 */
void set_listen_for_commands(bool val);

/**
 * Utilizes a lock guard to return whether we should continue to listen for commands.
 */
bool is_listening_for_commands();

#endif
