/**
 * @file http_websocket_server.c
 * @version 0.5.0
 *
 * @section License
 * Copyright (C) 2016, Erik Moqvist
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * This file is part of the Simba project.
 */

#include "simba.h"

int http_websocket_server_init(struct http_websocket_server_t *self_p,
                               struct socket_t *socket_p)
{
    self_p->socket_p = socket_p;

    return (0);
}

int http_websocket_server_handshake(struct http_websocket_server_t *self_p,
                                    struct http_server_request_t *request_p)
{
    char buf[160];
    char accept_key[29];
    const char *key_p;
    struct hash_sha1_t sha;
    uint8_t hash[20];
    size_t size;
    static const char secret[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    /* Only the GET action is supported. */
    if (request_p->action != http_server_request_action_get_t) {
        return (-1);
    }

    /* Sec-Websocket-Key is required. */
    if (request_p->headers.sec_websocket_key.present == 0) {
        log_object_print(NULL,
                         LOG_DEBUG,
                         FSTR("Missing HTTP header field: Sec-Websocket-Key\r\n"));

        return (-1);
    }

    /* Calculate the accept key. */
    key_p = request_p->headers.sec_websocket_key.value;
    size = strlen(key_p);
    memcpy(buf, key_p, size);
    memcpy(&buf[size], secret, sizeof(secret));

    hash_sha1_init(&sha);
    hash_sha1_update(&sha, buf, size + sizeof(secret) - 1);
    hash_sha1_digest(&sha, hash);

    base64_encode(accept_key, hash, sizeof(hash));
    accept_key[sizeof(accept_key) - 1] = '\0';
    
    /* Format and write the websocket handshake response to the
       client. */
    size = std_sprintf(buf,
                       FSTR("HTTP/1.1 101 Switching Protocols\r\n"
                            "Connection: Upgrade\r\n"
                            "Sec-WebSocket-Accept: %s\r\n"
                            "\r\n"),
                       accept_key);

    if (socket_write(self_p->socket_p, buf, size) != size) {
        return (-EIO);
    }

    return (0);
}

ssize_t http_websocket_server_read(struct http_websocket_server_t *self_p,
                                   int *type_p,
                                   void *buf_p,
                                   size_t size)
{
    uint8_t buf[16], *b_p = buf_p;
    size_t payload_left, left = size, n;
    int fin = 0;

    while (fin == 0) {
        /* Read the next frame. */
        if (socket_read(self_p->socket_p, buf, 2) != 2) {
            return (-EIO);
        }

        fin = (buf[0] & INET_HTTP_WEBSOCKET_FIN);
        payload_left = (buf[1] & ~INET_HTTP_WEBSOCKET_MASK);
        
        if (payload_left == 126) {
            if (socket_read(self_p->socket_p, &buf[2], 2) != 2) {
                return (-EIO);
            }
            
            payload_left = ((uint32_t)(buf[2]) << 8 | buf[3]);
        } else if (payload_left == 127) {
            if (socket_read(self_p->socket_p, &buf[2], 8) != 8) {
                return (-EIO);
            }
            
            payload_left = ((uint32_t)(buf[6]) << 24
                    | (uint32_t)(buf[7]) << 16
                    | (uint32_t)(buf[8]) << 8
                    | buf[9]);
        }
        
        /* Read the mask. */
        if (buf[1] & INET_HTTP_WEBSOCKET_MASK) {
            if (socket_read(self_p->socket_p, buf, 4) != 4) {
                return (-EIO);
            }
        }

        /* Read the payload. */
        while ((payload_left > 0) && (left > 0)) {
            if (payload_left < left) {
                n = payload_left;
            } else {
                n = left;
            }

            if (socket_read(self_p->socket_p, b_p, n) != n) {
                return (-1);
            }

            b_p += n;            
            left -= n;
            payload_left -= n;
        }

        /* Discard leftover data. */
        while (payload_left > 0) {
            if (socket_read(self_p->socket_p, buf, 1) != 1) {
                return (-1);
            }

            payload_left--;
        }
    }

    return (size - left);
}

ssize_t http_websocket_server_write(struct http_websocket_server_t *self_p,
                                    int type,
                                    const void *buf_p,
                                    size_t size)
{
    const uint8_t masking_key[4] = { 0x00, 0x00, 0x00, 0x00 };
    uint8_t header[16];
    size_t header_size = 2;
    
    header[0] = (INET_HTTP_WEBSOCKET_FIN | type);

    if (size < 126) {
        header[1] = (INET_HTTP_WEBSOCKET_MASK | size);
    } else if (size < 65536) {
        header[1] = (INET_HTTP_WEBSOCKET_MASK | 126);
        header[2] = ((size >> 8) & 0xff);
        header[3] = ((size >> 0) & 0xff);
        header_size += 2;
    } else {
        header[1] = (INET_HTTP_WEBSOCKET_MASK | 127);
        header[2] = 0;
        header[3] = 0;
        header[4] = 0;
        header[5] = 0;
        header[6] = ((size >> 24) & 0xff);
        header[7] = ((size >> 16) & 0xff);
        header[8] = ((size >>  8) & 0xff);
        header[9] = ((size >>  0) & 0xff);
        header_size += 8;
    }

    header[header_size + 0] = masking_key[0];
    header[header_size + 1] = masking_key[1];
    header[header_size + 2] = masking_key[2];
    header[header_size + 3] = masking_key[3];
    header_size += 4;

    if (socket_write(self_p->socket_p,
                     header,
                     header_size) != header_size) {
        return (-EIO);
    }

    if (socket_write(self_p->socket_p, buf_p, size) != size) {
        return (-EIO);
    }

    return (size);
}
