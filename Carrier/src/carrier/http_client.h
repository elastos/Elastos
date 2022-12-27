/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __HTTP_CLIENT_H__
#define __HTTP_CLIENT_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct http_client http_client_t;

typedef enum {
    HTTP_METHOD_HEAD,
    HTTP_METHOD_GET,
    HTTP_METHOD_POST,
    HTTP_METHOD_PUT,
    HTTP_METHOD_DELETE,
    HTTP_METHOD_PATCH
} http_method_t;

typedef enum {
    HTTP_VERSION_DEFAULT,
    HTTP_VERSION_1_0,
    HTTP_VERSION_1_1,
    HTTP_VERSION_2_0
} http_version_t;

const char *curl_strerror(int errcode);

const char *curlu_strerror(int errcode);

/*
 * Http client instance.
 */

http_client_t *http_client_new(void);

void http_client_close(http_client_t *client);
void http_client_reset(http_client_t *client);

/*
 * Http client options.
 */

int http_client_set_method(http_client_t *, http_method_t method);
int http_client_set_url(http_client_t *, const char *url);
int http_client_set_url_escape(http_client_t *, const char *url);
int http_client_set_path(http_client_t *, const char *path);
int http_client_set_query(http_client_t *, const char *name, const char *value);
int http_client_set_header(http_client_t *, const char *name, const char *value);
int http_client_set_timeout(http_client_t *, int timeout /* seconds */);
int http_client_set_version(http_client_t *, http_version_t version);

int http_client_get_url_escape(http_client_t *, char **url);
int http_client_get_scheme(http_client_t *, char **scheme);
int http_client_get_host(http_client_t *, char **host);
int http_client_get_port(http_client_t *, char **port);
int http_client_get_path(http_client_t *, char **path);

/*
 * Http client request/response body.
 */

#define HTTP_CLIENT_REQBODY_ABORT 0x10000000
typedef size_t (*http_client_request_body_callback_t)(char *buffer,
    size_t size, size_t nitems, void *userdata);

typedef size_t (*http_client_response_body_callback_t)(char *buffer,
    size_t size, size_t nitems, void *userdata);

typedef size_t (*http_client_response_header_callback_t)(char *buffer,
    size_t size, size_t nitems, void *userdata);

int http_client_set_request_body_instant(http_client_t *, void *data, size_t len);
int http_client_set_request_body(http_client_t *,
    http_client_request_body_callback_t cb, void *userdata);
int http_client_set_response_header(http_client_t *client,
    http_client_response_header_callback_t cb, void *userdata);
size_t http_client_get_response_header_length(http_client_t *);
int http_client_set_response_body(http_client_t *client,
    http_client_response_body_callback_t cb, void *userdata);
int http_client_enable_response_body(http_client_t *);
const char *http_client_get_response_body(http_client_t *);
size_t http_client_get_response_body_length(http_client_t *);
char *http_client_move_response_body(http_client_t *, size_t *len);
int http_client_get_response_code(http_client_t *, long *response_code);
int http_client_set_mime_instant(http_client_t *, const char *name,
                                 const char *filename, const char *type,
                                 const char *buffer, size_t bufsz);


/*
 * Http client request API
 */
int http_client_request(http_client_t *client);

/*
 * Escape/Unescape operation APIs.
 */
char *http_client_escape(http_client_t *client, const char *data, size_t len);

char *http_client_unescape(http_client_t *client, const char *data, size_t len,
    size_t *outlen);

void http_client_memory_free(void *ptr);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __HTTP_CLIENT_H__
