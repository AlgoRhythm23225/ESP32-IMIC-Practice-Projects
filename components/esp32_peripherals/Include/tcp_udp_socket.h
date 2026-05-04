#include "wifi_sta_ap.h"

// TCP socket
#define PORT_TCP 3333
void tcp_server_task(void *pvParameters);

// UDP socket
#define PORT_UDP 4023
void udp_server_task(void *pvParameters);