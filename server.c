#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netinet/tcp.h>

#include "websocket.h"
#include "database.h"
#include "logger.h"

#define PORT 8080
#define MAX_CLIENTS 50

int clients[MAX_CLIENTS];
int count = 0;
pthread_mutex_t lock;

/* 🔹 STRUCT */
typedef struct
{
    int sock;
    char ip[INET_ADDRSTRLEN];
} client_info;

/* 🔹 Broadcast */
void broadcast(char *msg)
{
    pthread_mutex_lock(&lock);

    for (int i = 0; i < count; i++)
    {
        websocket_send(clients[i], msg);
    }

    pthread_mutex_unlock(&lock);

    printf("[BROADCAST] %s\n", msg);
}

/* 🔹 HANDLE CLIENT */
void *handle(void *arg)
{
    client_info *c = (client_info *)arg;
    int sock = c->sock;
    char *ip = c->ip;

    char buffer[1024];
    char user[50], pass[50];

    printf("[SERVER] Client connected (%s)\n", ip);

    websocket_handshake(sock);

    /* LOGIN */
    while (1)
    {
        if (websocket_receive(sock, buffer) <= 0)
            continue;

        if (sscanf(buffer, "%s %s", user, pass) == 2)
            break;
    }

    printf("[LOGIN] %s (%s)\n", user, ip);

    if (!check_user(user, pass))
    {
        websocket_send(sock, "AUTH_FAIL");
        close(sock);
        free(c);
        return NULL;
    }

    websocket_send(sock, "AUTH_SUCCESS");
    int a, b;
    get_results(&a, &b);

    char result[100];
    sprintf(result, "RESULT %d %d", a, b);

    websocket_send(sock, result);
    /* VOTE */
    int vote = -1;

    while (1)
    {
        if (websocket_receive(sock, buffer) <= 0)
            continue;

        if (strcmp(buffer, "1") == 0 || strcmp(buffer, "2") == 0)
        {
            vote = atoi(buffer);
            break;
        }
    }

    if (check_voted(user))
    {
        websocket_send(sock, "ALREADY_VOTED");
        log_event("Duplicate vote", user, vote, ip);
        printf("[VOTE] Duplicate %s\n", user);
    }
    else
    {
        store_vote(user, vote);

        /* ADD LOGGING */
        log_event("Vote submitted", user, vote, ip);

        int a, b;
        get_results(&a, &b);

        char result[100];
        sprintf(result, "RESULT %d %d", a, b);

        /*  SEND RESULT TO CLIENT */
        websocket_send(sock, result);

        printf("[VOTE] %s voted %d (%s)\n", user, vote, ip);
    }
    sleep(1);
    close(sock);
    free(c);

    return NULL;
}
/* 🔹 MAIN */
int main()
{
    int server_fd, client;
    struct sockaddr_in addr;

    pthread_mutex_init(&lock, NULL);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    int flag = 1;
    setsockopt(server_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

    int keepalive = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 10);

    printf("WebSocket Server running on port %d...\n", PORT);

    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        client = accept(server_fd,
                        (struct sockaddr *)&client_addr,
                        &client_len);

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr,
                  client_ip, INET_ADDRSTRLEN);

        pthread_mutex_lock(&lock);
        clients[count++] = client;
        pthread_mutex_unlock(&lock);

        client_info *c = malloc(sizeof(client_info));
        c->sock = client;
        strcpy(c->ip, client_ip);

        pthread_t t;
        pthread_create(&t, NULL, handle, (void *)c);
    }
}