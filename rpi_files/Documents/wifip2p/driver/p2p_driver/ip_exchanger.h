#ifndef IP_EXCHANGER_HEADER
#define IP_EXCHANGER_HEADER

/**
 * Listens for a UDP (Datagram) packet from our connected p2p client.
 *
 * Will block until receiving a message, upon recieving this message, a response is sent to the client.
 * This response consists of the message "NLOS_CONNECTION_CONFIRM" followed by the IP address we have taken note of, forming the string:
 *
 * "NLOS_CONNECTION_CONFIRM xxx.xxx.xxx.xxx".
 *
 * If the client is using a wildcard IP to send out their initial message to us, it is their responsability to store the IP
 * this function sends in the confirmation message to ensure they recieve future messages from us.
 */
bool udp_connection_handshake();

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
bool tcp_connection_handshake();


#endif // IP_EXCHANGER_HEADER
