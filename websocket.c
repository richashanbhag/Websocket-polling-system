#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <sys/socket.h>
#include "websocket.h"

void websocket_handshake(int client)
{
    char buffer[2048];
    int bytes = recv(client, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0)
        return;

    buffer[bytes] = '\0';

    char *key_start = strstr(buffer, "Sec-WebSocket-Key:");
    if (!key_start)
        return;

    key_start += strlen("Sec-WebSocket-Key: ");

    char key[128], combined[256], base64[256], response[512];
    unsigned char sha1_hash[SHA_DIGEST_LENGTH];

    sscanf(key_start, "%127s", key);

    sprintf(combined,
            "%s258EAFA5-E914-47DA-95CA-C5AB0DC85B11", key);

    SHA1((unsigned char *)combined, strlen(combined), sha1_hash);

    EVP_EncodeBlock((unsigned char *)base64,
                    sha1_hash, SHA_DIGEST_LENGTH);

    sprintf(response,
            "HTTP/1.1 101 Switching Protocols\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Accept: %s\r\n\r\n",
            base64);

    send(client, response, strlen(response), 0);
}

int websocket_send(int client, char *msg)
{
    unsigned char frame[1024];
    int len = strlen(msg);

    frame[0] = 0x81;
    frame[1] = len;

    memcpy(frame + 2, msg, len);

    return send(client, frame, len + 2, 0);
}

/* 🔹 Robust receive */
int websocket_receive(int client, char *buffer)
{
    unsigned char frame[1024];
    int len = recv(client, frame, sizeof(frame), 0);

    if (len <= 0)
        return -1;

    int opcode = frame[0] & 0x0F;

    // 🔥 ONLY accept TEXT frames
    if (opcode != 0x1)
    {
        // ignore ping, pong, binary etc.
        return -1;
    }

    int payload_len = frame[1] & 127;
    int mask = frame[1] & 0x80;

    int offset = 2;

    if (payload_len == 126)
    {
        payload_len = (frame[2] << 8) | frame[3];
        offset = 4;
    }

    unsigned char *data;

    if (mask)
    {
        unsigned char *mask_key = &frame[offset];
        offset += 4;
        data = &frame[offset];

        for (int i = 0; i < payload_len; i++)
            buffer[i] = data[i] ^ mask_key[i % 4];
    }
    else
    {
        data = &frame[offset];
        memcpy(buffer, data, payload_len);
    }

    buffer[payload_len] = '\0';

    return payload_len;
}