#include "connect.h"
#include "ip_exchanger.h"
#include "command_handler.h"

#include <iostream>
#include <cstddef>
#include <wpa_ctrl.h>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>
#include <thread>

/**
 * A namespace which defines various constants with our P2P enviornment, such as filepaths, commands, etc.
 */
namespace P2PConstants {

    /******************** FILE PATHS ********************/

    /**
     * Path to the control interface which we will be monitoring.
     */
    const std::string CONTROL_PATH = "/var/run/wpa_supplicant/p2p-dev-wlan0";

    /**
     * Path to the start p2p script.
     */
    const std::string SCRIPT_START_P2P_SEARCH = "/home/pi/Documents/wifip2p/driver/scripts/runtime/start_fresh_p2p_search.sh";

    /**
     * Path to the script to dispose of our p2p environment.
     */
    const std::string SCRIPT_DISPOSE_P2P = "/home/pi/Documents/wifip2p/driver/scripts/runtime/dispose_p2p.sh";


    /******************** SYSTEM COMMAND STRING PARTS ********************/

    /**
     * Prefix to append to system commands involving P2P commands.
     */
    const std::string WPA_CLI_PREFIX = "wpa_cli -i p2p-dev-wlan0 ";

    /**
     * String containing the contents sudo bash.
     */
    const std::string SUDO_BASH = "sudo bash ";

    /**
     * P2P command to start searching for nearby devices.
     */
    const std::string P2P_FIND = "p2p_find ";

    /**
     * P2P command to connect to a device.
     */
    const std::string P2P_CONNECT = "p2p_connect ";

    /**
     * P2P connection type, virtual push button; append at the end of p2p_connect calls.
     */
    const std::string PBC = " pbc";



    /******************** P2P EVENT TYPES ********************/

    /**
     * String identical to the wpa_ctrl event message P2P_EVENT_GO_NEG_REQUEST.
     */
    const std::string GO_NEG_REQUEST = P2P_EVENT_GO_NEG_REQUEST;

    /**
     * String identical to the wpa_ctrl event message P2P_EVENT_GROUP_FORMATION_SUCCESS.
     */
    const std::string GROUP_FORMATION_SUCCESS = P2P_EVENT_GROUP_FORMATION_SUCCESS;

    /**
     * String identical to the wpa_ctrl event message P2P_EVENT_GROUP_FORMATION_FAILURE.
     */
    const std::string GROUP_FORMATION_FAILURE = P2P_EVENT_GROUP_FORMATION_FAILURE;

    /**
     * String identical to the wpa_ctrl event message P2P_EVENT_GROUP_STARTED.
     */
    const std::string GROUP_STARTED = P2P_EVENT_GROUP_STARTED;

    /**
     * String idential to the wpa_ctrl event message AP_STA_DISCONNECTED.
     */
    const std::string DEVICE_DISCONNECTED = AP_STA_DISCONNECTED;

    /**
     * The maximum amount of times to attempt to re-open or attach
     * to the wpa control interface.
     */
    const int MAX_TIMEOUT_ATTEMPTS = 5;

    /**
     * The length of a mac address in the format of:
     * xx:xx:xx:xx:xx:xx
     */
    static const int MAC_ADDRESS_LEN = 17;

    /**
     * The length of the prefix at the beginning of each wpa_ctrl message.
     * These tend to be of length 3; change to the length of the prefix on your machine.
     */
    static const size_t PREFIX_LEN = 3;

}

/********************* IP EXCHANGE METHOD *********************/

/**
 * The IP exchange function we will use to make our initial connection handshake.
 *
 * Ensure that this connection handshake is using the same method as the android app.
 *
 * Supported functions:
 * - tcp_connection_handshake (ip_exchanger.h)
 * - udp_connection_handshake (ip_exchanger.h)
 *
 */
static bool(*ip_exchange_fun)() = *udp_connection_handshake;


/********************* GLOBAL VARIABLES FOR MANIPULATION *********************/

/**
 * A static wpa_ctrl structure which is used by the funtions in this file to interact with the wpa_supplicant daemon.
 * If this is null, this is not registered as an event listener & thus the functions in this file will not work.
 */
static wpa_ctrl* control_data = nullptr;

/**
 * The name of the p2p group this device is the owner of.
 */
static std::string group_name = "";

/**
 * Indicates if our connection to the client device is valid.
 * This means an IP exchange has occured between us and the client.
 */
bool connection_is_valid = false;

/**
 * Main function. May prove to be useful to move this to its own non-main function in the future; for now will serve as
 * the main debugging function.
 */
int main() {
    //Initialize our connection to the wpa daemon:
    init_and_attach_ctrl();

    //If connection failed, end execution!
    if (control_data == nullptr)
        return -1;
    std::cout << "Successfully initialized the control interface." << std::endl;

    //Register termination signals so that no matter what happens to end the program, we clean up after ourselves.
    init_termination_signals();

    //Run a master loop. For now alternate between connection and connected loop.
    while (true) {
        //Start the loop notifying that we are available for connection!
        connect_loop();
        //While our UDP connection handshake fails, keep advertising as a P2P group owner.
        bool handshake_success = ip_exchange_fun();
        while (!handshake_success) {
            connect_loop();
            handshake_success = ip_exchange_fun();
        }
        enter_connected_state();
    }
    dispose_p2p();
    return 0;
}

/**
 * Enters the connected state of the program.
 * Spawns two threads:
 * - Thread 1: Will monitor the connection status of our P2P connection using function: connected_loop()
 * - Thread 2: Will open a socket and montior commands from the connected device.
 *
 * Termination of this function only occurs when the two threads have completed running.
 * Currently this only occurs when the device is disconnected from us or there has been an error opening the command handler socket.
 */
void enter_connected_state() {
    std::cout << "Entering connected state." << std::endl;
    set_listen_for_commands(true);
    std::thread command_handler(listen_for_commands);
    std::thread connection_listener(connected_loop);
    command_handler.join();
    connection_listener.join();
}

/********************* WPA_SUPPLICANT INIT AND DISPOSE FUNCTIONS *********************/

/**
 * Opens a WPA control interface and attempts to register the interface to recieve event messages.
 * On success, control_data field will be not null and available for use.
 * On failure, control_data field will be null and disposed of. No need to clean up.
 * In the case of an attach timeout, this method will attempt to open until it has reached an amount of attempts, MAX_TIMEOUT_ATTEMPTS
 * Check for null after calling this function!
 */
void init_and_attach_ctrl() {
    //If control data is not null, return! It is active.
    if (control_data != nullptr)
        return;
    control_data = wpa_ctrl_open(P2PConstants::CONTROL_PATH.c_str());
    if (control_data == nullptr) //If control data is null, return; failure opening interface.
        return;
    std::cout << "Successfully opened wpa_ctrl interface" << std::endl;
    //Attach to wpa daemon:
    int attach_status = -2;
    int attach_attempts = 0;
    while (attach_status == -2) { //-2 indicates a timeout. Attempt attach (or re-attach):
        attach_status = wpa_ctrl_attach(control_data);
        attach_attempts++;
        std::cout << "Attach attempt " << attach_attempts << " status = " << attach_status << std::endl;
        if (attach_attempts == P2PConstants::MAX_TIMEOUT_ATTEMPTS) //We have attempted to attact too many times at this point. Break loop.
            break;
    }
    if (attach_status != 0) { //0 indicates success, if attach wasn't successful; clean up after ourselves.
        wpa_ctrl_close(control_data);
        control_data = nullptr;
    }
}

/**
 * Disposes of the WPA control interface as well as terminates all P2P connections
 * this driver is responsible for handling.
 */
void dispose_p2p() {
    if (control_data == nullptr) { //If the control data structure is null there is no wpa_supplicant connection, return.
        std::cout << "Control data has already been disposed of." << std::endl;
        return;
    }
    wpa_ctrl_detach(control_data);
    wpa_ctrl_close(control_data);
    std::string terminate_p2p_command = P2PConstants::SUDO_BASH + P2PConstants::SCRIPT_DISPOSE_P2P;
    system(terminate_p2p_command.c_str());
    control_data = nullptr;
}

/********************* P2P CONNECTION FUNCTIONS *********************/

/**
 * Runs a loop which will listen for a P2P-GO-NEG-REQUEST from wpa_ctrl events.
 * Upon receiving a connection request, we allow the device to attempt to connect to us using the "virtual push button" connection method.
 * control_data MUST be initialized before calling this function for it to work properly!
 *
 * Function wipes ALL previously established P2P connections and events with the file SCRIPT_START_P2P_SEARCH!
 */
void connect_loop() {
    if (control_data == nullptr)
        return;
    bool connected = false;
    std::string start_p2p_command = P2PConstants::SUDO_BASH + P2PConstants::SCRIPT_START_P2P_SEARCH;
    system(start_p2p_command.c_str());
    while (!connected) { //While we aren't connected, listen for the wpa_ctrl system for incoming messages.
        if (wpa_ctrl_pending(control_data) == 1) { //If we recieved the return value 1, fetch the pending message.
            char message_buffer[256];
            size_t message_len = 256;
            if (recieve_message(message_buffer, &message_len)) { //If we have successfully recieved the message.
                if (is_message_type(message_buffer, message_len, P2PConstants::GO_NEG_REQUEST)) { //Check for a GO_NEG_REQUEST
                    std::string mac_address = get_mac_from_neg_request(message_buffer);
                    connected = connect_to_device(get_mac_from_neg_request(message_buffer));
                    //If we didn't successfully connect, restart the connection search.
                    if (!connected)
                        system(start_p2p_command.c_str());
                }
            }
        }
    }
}

/**
 * Runs a loop which will run while we are connected to a device. Will take note of P2P messages
 * we are interested in, such as connection status, group changes, etc.
 * In the case that we have disconnected from the device, this loop breaks.
 * In the case that we are no longer listening for commands, the loop will no longer run.
 */
void connected_loop() {
    bool connected = true;
    while (connected) {
        ///If we have a pending control message.
        if (wpa_ctrl_pending(control_data) == 1) {
            char message_buffer[256];
            size_t message_len = 256;
            if (recieve_message(message_buffer, &message_len)) {
                //AP_STA_DISCONNECTED:
                if (is_message_type(message_buffer, message_len, P2PConstants::DEVICE_DISCONNECTED)) {
                    //if the device disconnected, exit our loop and continue waiting.
                    std::cout << message_buffer << std::endl;
                    std::cout << "Device disconnected. Restarting P2P Connection search." << std::endl;
                    connection_is_valid = false;
                    set_listen_for_commands(false);
                    return;
                }
            }
        }
        ///If we are no longer listening for commands:
        if (!is_listening_for_commands()) {
            connection_is_valid = false; //Connection is not valid.
            std::cout << "Command socket is not listening. Disconnecting from device." << std::endl;
            return;
        }
    }
}


/**
 * Attempts to connect to a device given their mac address.
 * This function blocks until a connection has been established, or connection attempt fails.
 */
bool connect_to_device(std::string mac_address) {
    std::string connect_command = P2PConstants::WPA_CLI_PREFIX + P2PConstants::P2P_CONNECT + mac_address + P2PConstants::PBC;
    system(connect_command.c_str());
    while (true) { //Block until we hear about the connection status.
        if (wpa_ctrl_pending(control_data) == 1) {
            char message_buffer[256];
            size_t message_len = 256;
            if (recieve_message(message_buffer, &message_len)) {
                std::cout << message_buffer << std::endl;
                //Check for all of the message types we
                if (is_message_type(message_buffer, message_len, P2P_EVENT_GROUP_STARTED)) {
                    group_name = get_group_name_from_group_started(message_buffer);
                    std::cout << "Group started with name=" << group_name << std::endl;
                    return true;
                } else if (is_message_type(message_buffer, message_len, P2P_EVENT_GROUP_FORMATION_FAILURE) //If anyhting fails, a connection hasn't been established.
                                || is_message_type(message_buffer, message_len, P2P_EVENT_GO_NEG_FAILURE)) {
                    std::cout << "Aborting P2P Connection Attempt." << std::endl;
                    return false;
                }
            }
        }
    }
}

/**
 * Recieves a pending message from wpa_ctrl.
 * Returns true or false based on the message successfully being fetched.
 */
bool recieve_message(char* reply_buffer, size_t* reply_len) {
    int returnVal = wpa_ctrl_recv(control_data, reply_buffer, reply_len);
    return returnVal == 0;
}

/**
 * Determines if a wpa_ctrl message is of a message type.
 * NOTE: Does not take into account the prefix of each message; adjust PREFIX_LEN to the length of the prefix of your system.
 */
bool is_message_type(char* message, size_t message_len, std::string message_type) {
    //If the message length excluding the prefix to the message is shorter than the message type, there is no way they can be of the same type.
    if ((message_len - P2PConstants::PREFIX_LEN) < message_type.length())
        return false;
    //loop through the first n (message_type.length()) characters contained in the message
    for (size_t i = 0; i < message_type.length(); i++) {
        //If the characters aren't equal, return false.
        if (message[i + P2PConstants::PREFIX_LEN] != message_type.at(i))
            return false;
    }
    return true;
}

/**
 * Fetches the mac address from a P2P-GO-NEG-REQUEST.
 * Function is only guarenteed to work when it has been confirmed that this is a P2P-GO-NEG-REQUEST;
 * Ensure that it is by using the function is_message_type before using function.
 */
std::string get_mac_from_neg_request(char* message) {
    std::string mac = "";
    size_t index_offset = P2PConstants::PREFIX_LEN + P2PConstants::GO_NEG_REQUEST.length();
    for (size_t i = 0; i < P2PConstants::MAC_ADDRESS_LEN; i++) {
        mac += message[i + index_offset];
    }
    return mac;
}

/**
 * Fetches the name of a group from a P2P-GROUP-STARTED message.
 * Function is only guarenteed to work when it has been confirmed that this is a P2P-GROUP-STARTED;
 * Ensure that it is by using the function is_message_type before using function.
 */
std::string get_group_name_from_group_started(char* message) {
    std::string name = "";
    size_t index_offset = P2PConstants::PREFIX_LEN + P2PConstants::GROUP_STARTED.length();
    int i = 0;
    while (message[i + index_offset] != ' ') {
        name += message[i + index_offset];
        i++;
    }
    return name;
}

/********************* SIGNAL HANDLING FUNCTIONS *********************/

/**
 * Initializes this program to listen for any signals that an error has occured.
 * Ensures the proper disposal of the wpa event listener no matter what happens.
 */
void init_termination_signals() {
    signal(SIGABRT, signal_callback_handler);
    signal(SIGFPE, signal_callback_handler);
    signal(SIGILL, signal_callback_handler);
    signal(SIGINT, signal_callback_handler);
    signal(SIGSEGV, signal_callback_handler);
    signal(SIGTERM, signal_callback_handler);
}

/**
 * Handles ending our p2p service as well as exiting the program on recieving a signal.
 * This function should only be called via underlying system methods via registration with the function signal()!
 * Function's purpose is to handle cleaning upon any type of program termination.
 */
void signal_callback_handler(int signum) {
    std::cout << "\nCaught signal : " << signum << std::endl;
    dispose_p2p();
    exit(signum);
}
