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

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#include <curl/curl.h>
#include <crystal.h>

#include "http_client.h"

static long curl_http_versions[] = {
    CURL_HTTP_VERSION_NONE,
    CURL_HTTP_VERSION_1_0,
    CURL_HTTP_VERSION_1_1,
    CURL_HTTP_VERSION_2_0
};

typedef struct http_response_body {
    size_t used;
    size_t sz;
    void *data;
} http_response_body_t;

struct http_client {
    CURL *curl;
    CURLU *url;
    struct curl_slist *hdr;
    curl_mime *mime;
    http_response_body_t response_body;
};

static bool initialized = false;

const char *curl_strerror(int errcode)
{
    return curl_easy_strerror(errcode);
}

typedef struct ErrorDesc {
    int errcode;
    const char *errdesc;
} ErrorDesc;

static
const ErrorDesc curlu_codes[] = {
    { CURLUE_BAD_HANDLE,         "CURLU pointer argument passed as NULL." },
    { CURLUE_BAD_PARTPOINTER,    "part argument passed as NULL."          },
    { CURLUE_MALFORMED_INPUT,    "a malformed input passed."              },
    { CURLUE_BAD_PORT_NUMBER,    "invalid port number"                    },
    { CURLUE_UNSUPPORTED_SCHEME, "unsupported scheme"                     },
    { CURLUE_URLDECODE,          "URL decode error"                       },
    { CURLUE_OUT_OF_MEMORY,      "a memory function failed"               },
    { CURLUE_USER_NOT_ALLOWED,   "credentials was passed when prohibited" },
    { CURLUE_UNKNOWN_PART,       "an unknown part ID"                     },
    { CURLUE_NO_SCHEME,          "no scheme part in the URL"              },
    { CURLUE_NO_USER,            "no user part in the URL"                },
    { CURLUE_NO_PASSWORD,        "no password part in the URL"            },
    { CURLUE_NO_OPTIONS,         "no options part in the URL"             },
    { CURLUE_NO_HOST,            "no host part in the URL"                },
    { CURLUE_NO_PORT,            "no port part in the URL"                },
    { CURLUE_NO_QUERY,           "no query part in the URL"               },
    { CURLUE_NO_FRAGMENT,        "no fragment part in the URL"            }
};

const char *curlu_strerror(int errcode)
{
    int size = sizeof(curlu_codes)/sizeof(ErrorDesc);
    int i;

    for (i = 0; i < size; i++) {
        if (errcode == curlu_codes[i].errcode)
            return curlu_codes[i].errdesc;
    }

    return NULL;
}

static
void dump(const char *text, unsigned char *ptr, size_t size)
{
    size_t i;
    size_t c;
    unsigned int width=0x10;

    vlogV("HttpClient: %s, %10.10ld bytes (0x%8.8lx)",
          text, (long)size, (long)size);

    for(i=0; i<size; i+= width) {
        char buf[1024] = {0};
        char *cur = buf;

        sprintf(cur, "%4.4lx: ", (long)i);
        cur += strlen(cur);

        /* show hex to the left */
        for(c = 0; c < width; c++) {
            if(i+c < size)
                sprintf(cur, "%02x ", ptr[i+c]);
            else
                sprintf(cur, "   ");
            cur += strlen(cur);
        }

        /* show data on the right */
        for(c = 0; (c < width) && (i+c < size); c++) {
            char x = (ptr[i+c] >= 0x20 && ptr[i+c] < 0x80) ? ptr[i+c] : '.';
            *(cur++) = x;
        }
        *cur = '\0';

        vlogV("HttpClient: %s", buf);
    }
}

static
int trace_func(CURL *handle, curl_infotype type, char *data, size_t size,
               void *userp)
{
    const char *text;
    (void)handle; /* prevent compiler warning */
    (void)userp;

    if (log_level < VLOG_DEBUG)
        return 0;

    switch (type) {
    case CURLINFO_TEXT:
        vlogV("== Info: %s", data);
    default: /* in case a new one is introduced to shock us */
        return 0;

    case CURLINFO_HEADER_OUT:
        text = "=> Send header";
        break;
    case CURLINFO_DATA_OUT:
        text = "=> Send data";
        break;
    case CURLINFO_SSL_DATA_OUT:
        text = "=> Send SSL data";
        break;
    case CURLINFO_HEADER_IN:
        text = "<= Recv header";
        break;
    case CURLINFO_DATA_IN:
        text = "<= Recv data";
        break;
    case CURLINFO_SSL_DATA_IN:
        text = "<= Recv SSL data";
        break;
    }

    dump(text, (unsigned char *)data, size);
    return 0;
}

static size_t eat_output(char *ptr, size_t size,
                         size_t nmemb,
                         void *userdata)
{
    (void)ptr;
    (void)size;
    (void)nmemb;
    (void)userdata;
    return size * nmemb;
}

static void http_client_destroy(void *obj)
{
    http_client_t *client = (http_client_t *)obj;

    assert(client);

    if (client->response_body.data)
        free(client->response_body.data);
    if (client->curl)
        curl_easy_cleanup(client->curl);
    if (client->url)
        curl_url_cleanup(client->url);
    if (client->hdr)
        curl_slist_free_all(client->hdr);
    if (client->mime)
        curl_mime_free(client->mime);
}

http_client_t *http_client_new(void)
{
    http_client_t *client;

    if (!initialized) {
        int rc;
        rc = curl_global_init(CURL_GLOBAL_ALL);
        if (rc != CURLE_OK) {
            vlogE("HttpClient: Initialize global curl error (%d)", rc);
            return NULL;
        }
        initialized = true;
    }

    client = (http_client_t *)rc_zalloc(sizeof(http_client_t), http_client_destroy);
    if (!client)
        return NULL;

    client->url = curl_url();
    if (!client->url) {
        vlogE("HttpClient: curl_url() failure.");
        deref(client);
        return NULL;
    }

    client->curl = curl_easy_init();
    if (!client->curl) {
        vlogE("HttpClient: curl_easy_init() failure.");
        deref(client);
        return NULL;
    }

    curl_easy_setopt(client->curl, CURLOPT_DEBUGFUNCTION, trace_func);
    curl_easy_setopt(client->curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(client->curl, CURLOPT_WRITEFUNCTION, eat_output);
    curl_easy_setopt(client->curl, CURLOPT_CURLU, client->url);
    curl_easy_setopt(client->curl, CURLOPT_NOSIGNAL, 1L);
#if defined(_WIN32) || defined(_WIN64)
    curl_easy_setopt(client->curl, CURLOPT_SSL_VERIFYPEER, 0);
#endif

    return client;
}

void http_client_close(http_client_t *client)
{
    if (client)
        deref(client);
}

void http_client_reset(http_client_t *client)
{
    assert(client);

    curl_easy_reset(client->curl);
    curl_url_set(client->url, CURLUPART_URL, NULL, 0);

    if (client->hdr) {
        curl_slist_free_all(client->hdr);
        client->hdr = NULL;
    }

    if (client->mime) {
        curl_mime_free(client->mime);
        client->mime = NULL;
    }

    client->response_body.used = 0;

    curl_easy_setopt(client->curl, CURLOPT_DEBUGFUNCTION, trace_func);
    curl_easy_setopt(client->curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(client->curl, CURLOPT_WRITEFUNCTION, eat_output);
    curl_easy_setopt(client->curl, CURLOPT_CURLU, client->url);
    curl_easy_setopt(client->curl, CURLOPT_NOSIGNAL, 1L);
#if defined(_WIN32) || defined(_WIN64)
    curl_easy_setopt(client->curl, CURLOPT_SSL_VERIFYPEER, 0);
#endif
}

int http_client_set_method(http_client_t *client, http_method_t method)
{
    CURLcode code = CURLE_UNSUPPORTED_PROTOCOL;

    switch (method) {
    case HTTP_METHOD_HEAD:
        code = curl_easy_setopt(client->curl, CURLOPT_NOBODY, 1L);
        break;
    case HTTP_METHOD_GET:
        code = curl_easy_setopt(client->curl, CURLOPT_HTTPGET, 1L);
        break;
    case HTTP_METHOD_POST:
        code = curl_easy_setopt(client->curl, CURLOPT_HTTPPOST, 1L);
        break;
    case HTTP_METHOD_PUT:
        code = curl_easy_setopt(client->curl, CURLOPT_CUSTOMREQUEST, "PUT");
        break;
    case HTTP_METHOD_DELETE:
        code = curl_easy_setopt(client->curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        break;
    case HTTP_METHOD_PATCH:
        code = curl_easy_setopt(client->curl, CURLOPT_CUSTOMREQUEST, "PATCH");
        break;
    default:
        assert(0);
        break;
    }

    return code;
}

int http_client_set_url(http_client_t *client, const char *url)
{
    CURLUcode code;

    assert(client);
    assert(url);
    assert(*url);

    code = curl_url_set(client->url, CURLUPART_URL, url, CURLU_URLENCODE);
    if (code != CURLUE_OK) {
        vlogE("HttpClient: Set url %s error (%d)", url, code);
        return code;
    }

    return 0;
}

int http_client_set_url_escape(http_client_t *client, const char *url)
{
    CURLUcode code;

    assert(client);
    assert(url);
    assert(*url);

    code = curl_url_set(client->url, CURLUPART_URL, url, 0);
    if (code != CURLUE_OK) {
        vlogE("HttpClient: Escape url %s error (%d)", url, code);
        return code;
    }

    return 0;
}

int http_client_get_url_escape(http_client_t *client, char **url)
{
    CURLUcode code;

    assert(client);
    assert(url);

    code = curl_url_get(client->url, CURLUPART_URL, url, 0);
    if (code != CURLUE_OK)  {
        vlogE("HttpClient: Get url from curl error (%d)", code);
        return code;
    }

    return 0;
}

int http_client_get_scheme(http_client_t *client, char **scheme)
{
    CURLUcode code;

    assert(client);
    assert(scheme);

    code = curl_url_get(client->url, CURLUPART_SCHEME, scheme, CURLU_URLDECODE);
    if (code != CURLUE_OK)  {
        vlogE("HttpClient: Get url from curl error (%d)", code);
        return code;
    }

    return 0;
}

int http_client_get_host(http_client_t *client, char **host)
{
    CURLUcode code;

    assert(client);
    assert(host);

    code = curl_url_get(client->url, CURLUPART_HOST, host, CURLU_URLDECODE);
    if (code != CURLUE_OK)  {
        vlogE("HttpClient: Get url from curl error (%d)", code);
        return  code;
    }

    return 0;
}

int http_client_get_port(http_client_t *client, char **port)
{
    CURLUcode code;

    assert(client);
    assert(port);

    code = curl_url_get(client->url, CURLUPART_PORT, port, CURLU_URLDECODE);
    if (code != CURLUE_OK)  {
        vlogE("HttpClient: Get url from curl error (%d)", code);
        return code;
    }

    return 0;
}

int http_client_get_path(http_client_t *client, char **path)
{
    CURLUcode code;

    assert(client);
    assert(path);

    code = curl_url_get(client->url, CURLUPART_PATH, path, CURLU_URLDECODE);
    if (code != CURLUE_OK)  {
        vlogE("HttpClient: Get url from curl error (%d)", code);
        return code;
    }

    return 0;
}

int http_client_set_path(http_client_t *client, const char *path)
{
    CURLUcode code;

    assert(client);
    assert(path);
    assert(*path);

    code = curl_url_set(client->url, CURLUPART_PATH, path, CURLU_URLENCODE);
    if (code != CURLUE_OK) {
        vlogE("HttpClient: Set path from curl error (%d)", code);
        return code;
    }

    return 0;
}

int http_client_set_query(http_client_t *client,
                          const char *name, const char *value)
{
    char *query;
    CURLUcode code;

    assert(client);
    assert(name);
    assert(value);
    assert(*name);
    assert(*value);

    query = alloca(strlen(name) + strlen(value) + 2);
    sprintf(query, "%s=%s", name, value);

    code = curl_url_set(client->url, CURLUPART_QUERY, query,
                        CURLU_URLENCODE | CURLU_APPENDQUERY);
    if (code != CURLUE_OK) {
        vlogE("HttpClient: Set query from curl error (%d)", code);
        return code;
    }

    return 0;
}

int http_client_set_header(http_client_t *client,
                           const char *name, const char *value)
{
    char *header;
    struct curl_slist *hdr;

    assert(client);
    assert(name);
    assert(value);
    assert(*name);

    header = alloca(strlen(name) + strlen(value) + 3);
    sprintf(header, "%s: %s", name, value);

    hdr = curl_slist_append(client->hdr, header);
    if (!hdr) {
        vlogE("HttpClient: Set header from curl error");
        return CURLE_OUT_OF_MEMORY;
    }

    client->hdr = hdr;
    return 0;
}

int http_client_set_timeout(http_client_t *client, int timeout)
{
    CURLcode code;

    assert(client);
    assert(timeout > 0);

    code = curl_easy_setopt(client->curl, CURLOPT_TIMEOUT_MS, timeout);
    if (code != CURLE_OK)
        return code;

    code = curl_easy_setopt(client->curl, CURLOPT_CONNECTTIMEOUT_MS, timeout);
    if (code != CURLE_OK)
        return code;

    return 0;
}

int http_client_set_version(http_client_t *client, http_version_t version)
{
    CURLcode code;

    assert(client);
    assert(version >= CURL_HTTP_VERSION_NONE);
    assert(version <= CURL_HTTP_VERSION_2_0);

    code = curl_easy_setopt(client->curl, CURLOPT_HTTP_VERSION,
                            curl_http_versions[version]);

    return code;
}

int http_client_set_request_body_instant(http_client_t *client,
                                         void *data, size_t len)
{
    assert(client);

    curl_easy_setopt(client->curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(client->curl, CURLOPT_POSTFIELDSIZE, len);

    return 0;
}

int http_client_set_request_body(http_client_t *client,
                                 http_client_request_body_callback_t callback,
                                 void *userdata)
{
    assert(client);
    assert(callback);

    curl_easy_setopt(client->curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(client->curl, CURLOPT_READFUNCTION, callback);
    curl_easy_setopt(client->curl, CURLOPT_READDATA, userdata);

    return 0;
}

int http_client_set_response_body(http_client_t *client,
                                  http_client_response_body_callback_t callback,
                                  void *userdata)
{
    assert(client);
    assert(callback);

    curl_easy_setopt(client->curl, CURLOPT_WRITEFUNCTION, callback);
    curl_easy_setopt(client->curl, CURLOPT_WRITEDATA, userdata);

    return 0;
}

static size_t http_response_body_write_callback(char *ptr, size_t size, size_t nmemb,
                                                void *userdata)
{
    http_response_body_t *response = (http_response_body_t *)userdata;
    size_t length = size * nmemb;

    if (response->sz - response->used < length) {
        size_t new_sz;
        size_t last_try;
        void *new_data;

        if (response->sz + length < response->sz) {
            response->used = 0;
            return 0;
        }

        for (new_sz = response->sz ? response->sz << 1 : 512, last_try = response->sz;
            new_sz > last_try && new_sz <= response->sz + length;
            last_try = new_sz, new_sz <<= 1) ;

        if (new_sz <= last_try)
            new_sz = response->sz + length;

        new_data = realloc(response->data, new_sz);
        if (!new_data) {
            response->used = 0;
            return 0;
        }

        response->data = new_data;
        response->sz = new_sz;
    }

    memcpy((char *)response->data + response->used, ptr, length);
    response->used += length;

    return length;
}

int http_client_enable_response_body(http_client_t *client)
{
    curl_easy_setopt(client->curl, CURLOPT_WRITEFUNCTION,
                     http_response_body_write_callback);

    curl_easy_setopt(client->curl, CURLOPT_WRITEDATA,
                     &client->response_body);
    return 0;
}

const char *http_client_get_response_body(http_client_t *client)
{
    return client->response_body.used ? client->response_body.data : NULL;
}

char *http_client_move_response_body(http_client_t *client, size_t *len)
{
    char *resp;

    if (!client->response_body.used) {
        *len = 0;
        return NULL;
    }

    if (len)
        *len = client->response_body.used;
    resp = client->response_body.data;

    client->response_body.data = NULL;
    client->response_body.used = 0;
    client->response_body.sz = 0;

    return resp;
}

size_t http_client_get_response_body_length(http_client_t *client)
{
    return client->response_body.used;
}

int http_client_set_response_header(http_client_t *client,
                                    http_client_response_header_callback_t callback,
                                    void *userdata)
{
    assert(client);
    assert(callback);

    curl_easy_setopt(client->curl, CURLOPT_HEADERFUNCTION, callback);
    curl_easy_setopt(client->curl, CURLOPT_HEADERDATA, userdata);

    return 0;
}

int http_client_request(http_client_t *client)
{
    CURLcode code;

    assert(client);

    if (client->hdr)
        curl_easy_setopt(client->curl, CURLOPT_HTTPHEADER, client->hdr);

    if (client->mime)
        curl_easy_setopt(client->curl, CURLOPT_MIMEPOST, client->mime);


    curl_easy_setopt(client->curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(client->curl, CURLOPT_SSL_VERIFYHOST, 0L);
    code = curl_easy_perform(client->curl);
    if (code != CURLE_OK) {
        vlogE("HttpClient: Perform request error (%d)", code);
        return code;
    }

    return 0;
}

int http_client_get_response_code(http_client_t *client, long *response_code)
{
    CURLcode code;

    assert(client);
    assert(response_code);

    code = curl_easy_getinfo(client->curl, CURLINFO_RESPONSE_CODE,
                             response_code);
    if (code != CURLE_OK) {
        vlogE("HttpClient: Get http response code error (%d)", code);
        return code;
    }

    return 0;
}

char *http_client_escape(http_client_t *client, const char *data, size_t len)
{
    char *escaped_data;

    assert(client);
    assert(data);
    assert(len);

    escaped_data = curl_easy_escape(client->curl, data, (int)len);
    if (!escaped_data) {
        vlogE("HttpClient: Escape data error");
        return NULL;
    }

    return escaped_data;
}

char *http_client_unescape(http_client_t *client, const char *data, size_t len,
                           size_t *outlen)
{
    char *unescaped_data;
    int _outlen = 0;

    assert(client);
    assert(data);
    assert(len);
    assert(outlen);

    unescaped_data = curl_easy_unescape(client->curl, data, (int)len, &_outlen);
    if (!unescaped_data) {
        vlogE("HttpClient: Unescape data error");
        return NULL;
    }

    *outlen = (size_t)_outlen;
    return unescaped_data;
}

void http_client_memory_free(void *ptr)
{
    curl_free(ptr);
}

int http_client_set_mime_instant(http_client_t *client, const char *name,
                                 const char *filename, const char *type,
                                 const char *buffer, size_t bufsz)
{
    curl_mimepart *part;

    if (!client->mime)
        client->mime = curl_mime_init(client->curl);

    part = curl_mime_addpart(client->mime);
    curl_mime_name(part, name);
    curl_mime_filename(part, filename);
    curl_mime_type(part,type);
    curl_mime_data(part, buffer, bufsz);

    return 0;
}
