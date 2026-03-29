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

/* 🔹 STRUCT for client */
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

/* 🔹 Handle client */
void *handle(void *arg)
{
    client_info *c = (client_info *)arg;
    int sock = c->sock;
    char *ip = c->ip;

    char buffer[1024];
    char user[50] = "unknown";

    printf("[SERVER] Client connected (%s)\n", ip);

    /* 🔹 Handshake */
    websocket_handshake(sock);
    printf("[HANDSHAKE] Completed (%s)\n", ip);

    /* 🔹 LOGIN */
    if (websocket_receive(sock, buffer) <= 0)
    {
        close(sock);
        free(c);
        return NULL;
    }

    char pass[50];
    sscanf(buffer, "%s %s", user, pass);

    printf("[LOGIN ATTEMPT] %s (%s)\n", user, ip);

    if (!check_user(user, pass))
    {
        websocket_send(sock, "AUTH_FAIL");
        printf("[AUTH] Failed (%s)\n", user);

        log_event("Login failed", user, -1, ip);

        close(sock);
        free(c);
        return NULL;
    }

    websocket_send(sock, "AUTH_SUCCESS");
    log_event("User login", user, -1, ip);

    printf("[AUTH] Success: %s (%s)\n", user, ip);

    /* 🔹 VOTE */
    if (websocket_receive(sock, buffer) <= 0)
    {
        close(sock);
        free(c);
        return NULL;
    }

    int vote = atoi(buffer);

    if (check_voted(user))
    {
        websocket_send(sock, "ALREADY_VOTED");

        printf("[VOTE] Duplicate attempt by %s (%s)\n", user, ip);

        log_event("Duplicate vote attempt", user, vote, ip);
    }
    else
    {
        store_vote(user, vote);

        printf("[VOTE] %s voted %d (%s)\n", user, vote, ip);

        log_event("Vote submitted", user, vote, ip);

        int a, b;
        get_results(&a, &b);

        char result[100];
        sprintf(result, "RESULT %d %d", a, b);

        broadcast(result);
    }

    /* 🔹 DISCONNECT */
    log_event("User disconnected", user, -1, ip);

    printf("[DISCONNECT] %s (%s)\n", user, ip);

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