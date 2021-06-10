#ifndef CONNECT_HEADER
#define CONNECT_HEADER

#include <string>

/******************** FUNCTION DECLARATIONS ********************/

/**
 * Enters the connected state of the program.
 * Spawns two threads:
 * - Thread 1: Will monitor the connection status of our P2P connection using function: connected_loop()
 * - Thread 2: Will open a socket and montior commands from the connected device using function: listen_for_commands() (command_handler.h)
 *
 * Termination of this function only occurs when the two threads have completed running.
 * Currently this only occurs when the device is disconnected from us.
 */
void enter_connected_state();

/**
 * Opens a WPA control interface and attempts to register the interface to recieve event messages.
 * On success, control_data field will be not null and available for use.
 * On failure, control_data field will be null and disposed of. No need to clean up.
 * In the case of an attach timeout, this method will attempt to open until it has reached an amount of attempts, MAX_TIMEOUT_ATTEMPTS
 * Check for null after calling this function!
 */
void init_and_attach_ctrl();

/**
 * Disposes of the WPA control interface as well as terminates all P2P connections
 * this driver is responsible for handling.
 */
void dispose_p2p();

/**
 * Initializes this program to listen for any signals that an error has occured.
 * Ensures the proper disposal of the wpa event listener no matter what happens.
 */
void init_termination_signals();

/**
 * Handles ending our p2p service as well as exiting the program on recieving a signal.
 * This function should only be called via underlying system methods via registration with the function signal()!
 * Function's purpose is to handle cleaning upon any type of program termination.
 */
void signal_callback_handler(int signum);

/**
 * Runs a loop which will listen for a P2P-GO-NEG-REQUEST from wpa_ctrl events.
 * Upon receiving a connection request, we allow the device to attempt to connect to us using the "virtual push button" connection method.
 * control_data MUST be initialized before calling this function for it to work properly!
 */
void connect_loop();

/**
 * Runs a loop which will take note of various P2P notifications, such as
 */
void connected_loop();

/**
 * Attempts to connect to a device given their mac address.
 * This function blocks until a connection has been established, or connection attempt fails.
 */
bool connect_to_device(std::string mac_address);

/**
 * Recieves a pending message from wpa_ctrl.
 * Returns true or false based on the message successfully being fetched.
 */
bool recieve_message(char* reply_buffer, size_t* reply_len);

/**
 * Determines if a wpa_ctrl message is of a message type.
 * NOTE: Does not take into account the prefix of each message; adjust PREFIX_LEN to the length of the prefix of your system.
 */
bool is_message_type(char* message, size_t message_len, std::string message_type);

/**
 * Fetches the mac address from a P2P-GO-NEG-REQUEST.
 * Function is only guarenteed to work when it has been confirmed that this is a P2P-GO-NEG-REQUEST;
 * Ensure that it is by using the function is_message_type before using function.
 */
std::string get_mac_from_neg_request(char* message);

/**
 * Fetches the name of a group from a P2P-GROUP-STARTED message.
 * Function is only guarenteed to work when it has been confirmed that this is a P2P-GROUP-STARTED;
 * Ensure that it is by using the function is_message_type before using function.
 */
std::string get_group_name_from_group_started(char* message);


#endif // CONNECT_HEADER
