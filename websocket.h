#ifndef WEBSOCKET_H
#define WEBSOCKET_H

void websocket_handshake(int client);
int websocket_send(int client, char *msg);
int websocket_receive(int client, char *buffer);

#endif