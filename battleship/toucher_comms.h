#ifndef TOUCHER_COMMS_H
#define TOUCHER_COMMS_H

int send_message(char *message) ;

int server_init();
int client_init(char *server_ip);

#endif