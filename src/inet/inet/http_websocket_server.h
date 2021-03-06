/**
 * @file inet/http_websocket_server.h
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
 * MERBITSTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * This file is part of the Simba project.
 */

#ifndef __INET_HTTP_WEBSOCKET_SERVER_H__
#define __INET_HTTP_WEBSOCKET_SERVER_H__

#include "simba.h"

#define HTTP_TYPE_TEXT   1
#define HTTP_TYPE_BINARY 2

struct http_websocket_server_t {
    struct socket_t *socket_p;
};

/**
 * Initialize given websocket server. The server uses the http module
 * interface to communicate with the client.
 *
 * @param[in] self_p Http to initialize.
 * @param[in] socket_p Connected socket.
 *
 * @return zero(0) or negative error code.
 */
int http_websocket_server_init(struct http_websocket_server_t *self_p,
                               struct socket_t *socket_p);

/**
 * Read the handshake request from the client and send the handshake
 * response.
 *
 * @param[in] self_p Websocket server.
 * @param[in] request_p Read handshake request.
 *
 * @return zero(0) or negative error code.
 */
int http_websocket_server_handshake(struct http_websocket_server_t *self_p,
                                    struct http_server_request_t *request_p);

/**
 * Read a message from given websocket.
 *
 * @param[in] self_p Websocket to read from.
 * @param[out] type_p Read message type.
 * @param[in] buf_p Buffer to read into.
 * @param[in] size Number of bytes to read. Longer messages will be
 *                 truncated and the leftover data dropped.
 *
 * @return Number of bytes read or negative error code.
 */
ssize_t http_websocket_server_read(struct http_websocket_server_t *self_p,
                                   int *type_p,
                                   void *buf_p,
                                   size_t size);

/**
 * Write given message to given websocket.
 *
 * @param[in] self_p Websocket to write to.
 * @param[in] type One of ``HTTP_TYPE_TEXT`` and ``HTTP_TYPE_BINARY``.
 * @param[in] buf_p Buffer to write.
 * @param[in] size Number of bytes to write.
 *
 * @return Number of bytes written or negative error code.
 */
ssize_t http_websocket_server_write(struct http_websocket_server_t *self_p,
                                    int type,
                                    const void *buf_p,
                                    size_t size);

#endif
