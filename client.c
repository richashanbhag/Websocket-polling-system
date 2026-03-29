#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

void send_websocket_frame(int sock, char *msg)
{
    unsigned char frame[1024];
    int len = strlen(msg);

    frame[0] = 0x81;       // text frame
    frame[1] = len | 0x80; // masked

    unsigned char mask[4] = {1, 2, 3, 4};
    memcpy(frame + 2, mask, 4);

    for (int i = 0; i < len; i++)
    {
        frame[6 + i] = msg[i] ^ mask[i % 4];
    }

    send(sock, frame, len + 6, 0);
}
int websocket_read(int sock, char *buffer)
{
    unsigned char frame[1024];
    int len = recv(sock, frame, sizeof(frame), 0);

    if (len <= 0)
        return -1;

    int payload_len = frame[1] & 127;

    memcpy(buffer, frame + 2, payload_len);
    buffer[payload_len] = '\0';

    return payload_len;
}

int main()
{
    int sock;
    struct sockaddr_in server;
    char buffer[2048];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (struct sockaddr *)&server, sizeof(server));

    // 🔹 WebSocket Handshake Request
    char *handshake =
        "GET / HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: abc123==\r\n"
        "Sec-WebSocket-Version: 13\r\n\r\n";

    send(sock, handshake, strlen(handshake), 0);

    recv(sock, buffer, sizeof(buffer), 0);
    printf("Handshake completed\n");

    // 🔹 Send Login
    char user[50], pass[50];
    printf("Username: ");
    scanf("%s", user);
    printf("Password: ");
    scanf("%s", pass);

    char msg[100];
    sprintf(msg, "%s %s", user, pass);
    send_websocket_frame(sock, msg);

    websocket_read(sock, buffer);
    printf("Server: %s\n", buffer);

    // 🔹 Send Vote
    int vote;
    printf("Vote (1/2): ");
    scanf("%d", &vote);

    sprintf(msg, "%d", vote);
    send_websocket_frame(sock, msg);

    websocket_read(sock, buffer);
    printf("Result: %s\n", buffer);

    close(sock);
    return 0;
}