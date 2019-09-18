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

#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#include <crystal.h>
#include <cjson/cJSON.h>

#include "dstore.h"
#include "http_client.h"

#define MAX_IPV4_ADDRESS_LEN (15)
#define MAX_IPV6_ADDRESS_LEN (47)

#define HttpStatus_OK 200

#define DSTORE_UID "uid-2041b18e-ca86-4962-9a21-d477f7f627ce"

#define MSG_PATH "/messages"

typedef struct rpc_node {
    char ipv4[MAX_IPV4_ADDRESS_LEN + 1];
    char ipv6[MAX_IPV6_ADDRESS_LEN + 1];
    uint16_t port;
} rpc_node_t;

struct DStore {
    char current_node_ip[MAX_IPV6_ADDRESS_LEN + 1];
    uint16_t current_node_port;

    size_t rpc_nodes_count;
    rpc_node_t rpc_nodes[0];
};

static inline bool has_working_node(DStore *dstore)
{
    return *dstore->current_node_ip ? true : false;
}

static inline void clear_working_node(DStore *dstore)
{
    memset(dstore->current_node_ip, 0, sizeof(dstore->current_node_ip));
}

static int configure_working_node(DStore *dstore);

static int ipfs_get_node_version(const char *node_ip, uint16_t node_port)
{
    char url[MAXPATHLEN + 1] = {0};
    http_client_t *httpc;
    long resp_code = 0;
    int rc;

    rc = snprintf(url, sizeof(url), "http://%s:%u/version",
                  node_ip, (unsigned)node_port);
    if (rc < 0 || rc >= sizeof(url))
        return -1;

    httpc = http_client_new();
    if (!httpc)
        return -1;

    http_client_set_url(httpc, url);
    http_client_set_method(httpc, HTTP_METHOD_POST);
    http_client_set_request_body_instant(httpc, NULL, 0);
    http_client_set_timeout(httpc, 5);

    rc = http_client_request(httpc);
    if (rc != 0)
        goto error_exit;

    rc = http_client_get_response_code(httpc, &resp_code);
    http_client_close(httpc);

    if (rc != 0)
        return -1;

    if (resp_code != HttpStatus_OK)
        return -1;

    return 0;

error_exit:
    http_client_close(httpc);
    return -1;
}

static int ipfs_get_uid_info(DStore *dstore, const char *uid, char **resp)
{
    char url[MAXPATHLEN + 1] = {0};
    http_client_t *httpc;
    long resp_code = 0;
    char *p;
    int rc;

    assert(resp);

    if (!has_working_node(dstore) && configure_working_node(dstore) < 0)
        return -1;

    rc = snprintf(url, sizeof(url), "http://%s:%u/api/v0/uid/info",
                  dstore->current_node_ip, (unsigned)dstore->current_node_port);
    if (rc < 0 || rc >= sizeof(url))
        return -1;

    httpc = http_client_new();
    if (!httpc)
        return -1;

    http_client_set_url(httpc, url);
    http_client_set_query(httpc, "uid", uid);
    http_client_set_method(httpc, HTTP_METHOD_POST);
    http_client_set_request_body_instant(httpc, NULL, 0);
    http_client_enable_response_body(httpc);

    rc = http_client_request(httpc);
    if (rc != 0) {
        clear_working_node(dstore);
        goto error_exit;
    }

    rc = http_client_get_response_code(httpc, &resp_code);
    if (rc != 0)
        goto error_exit;

    if (resp_code != HttpStatus_OK)
        goto error_exit;

    p = http_client_move_response_body(httpc, NULL);
    http_client_close(httpc);

    if (!p)
        return -1;

    *resp = p;
    return 0;

error_exit:
    http_client_close(httpc);
    return -1;
}

static int ipfs_resolve(DStore *dstore, const char *peerid, char **resp)
{
    char url[MAXPATHLEN + 1] = {0};
    http_client_t *httpc;
    long resp_code = 0;
    char *p;
    int rc;

    assert(resp);

    if (!has_working_node(dstore) && configure_working_node(dstore) < 0)
        return -1;

    rc = snprintf(url, sizeof(url), "http://%s:%u/api/v0/name/resolve",
                  dstore->current_node_ip, (unsigned)dstore->current_node_port);
    if (rc < 0 || rc >= sizeof(url))
        return -1;

    httpc = http_client_new();
    if (!httpc)
        return -1;

    http_client_set_url(httpc, url);
    http_client_set_query(httpc, "arg", peerid);
    http_client_set_method(httpc, HTTP_METHOD_GET);
    http_client_enable_response_body(httpc);

    rc = http_client_request(httpc);
    if (rc != 0) {
        clear_working_node(dstore);
        goto error_exit;
    }

    rc = http_client_get_response_code(httpc, &resp_code);
    if (rc != 0)
        goto error_exit;

    if (resp_code != HttpStatus_OK)
        goto error_exit;

    p = http_client_move_response_body(httpc, NULL);
    http_client_close(httpc);

    if (!p)
        return -1;

    *resp = p;
    return 0;

error_exit:
    http_client_close(httpc);
    return -1;
}

static int ipfs_login(DStore *dstore, const char *uid, const char *root_hash)
{
    char url[MAXPATHLEN + 1] = {0};
    http_client_t *httpc;
    long resp_code = 0;
    int rc;

    if (!has_working_node(dstore) && configure_working_node(dstore) < 0)
        return -1;

    rc = snprintf(url, sizeof(url), "http://%s:%u/api/v0/uid/login",
                  dstore->current_node_ip, (unsigned)dstore->current_node_port);
    if (rc < 0 || rc >= sizeof(url))
        return -1;

    httpc = http_client_new();
    if (!httpc)
        return -1;

    http_client_set_url(httpc, url);
    http_client_set_query(httpc, "uid", uid);
    http_client_set_query(httpc, "hash", root_hash);
    http_client_set_method(httpc, HTTP_METHOD_POST);
    http_client_set_request_body_instant(httpc, NULL, 0);

    rc = http_client_request(httpc);
    if (rc != 0) {
        clear_working_node(dstore);
        goto error_exit;
    }

    rc = http_client_get_response_code(httpc, &resp_code);
    http_client_close(httpc);

    if (rc != 0)
        return -1;

    if (resp_code != HttpStatus_OK)
        return -1;

    return 0;

error_exit:
    http_client_close(httpc);
    return -1;
}

static int ipfs_list_files(DStore *dstore, const char *uid,
                           const char *path, char **resp)
{
    char url[MAXPATHLEN + 1] = {0};
    http_client_t *httpc;
    long resp_code;
    char *p;
    int rc;

    assert(resp);

    if (!has_working_node(dstore) && configure_working_node(dstore) < 0)
        return -1;

    rc = snprintf(url, sizeof(url), "http://%s:%u/api/v0/files/ls",
                  dstore->current_node_ip, (unsigned)dstore->current_node_port);
    if (rc < 0 || rc >= sizeof(url))
        return -1;

    httpc = http_client_new();
    if (!httpc)
        return -1;

    http_client_set_url(httpc, url);
    http_client_set_query(httpc, "uid", uid);
    http_client_set_query(httpc, "path", path);
    http_client_set_method(httpc, HTTP_METHOD_POST);
    http_client_set_request_body_instant(httpc, NULL, 0);
    http_client_enable_response_body(httpc);

    rc = http_client_request(httpc);
    if (rc != 0) {
        clear_working_node(dstore);
        goto error_exit;
    }

    rc = http_client_get_response_code(httpc, &resp_code);
    if (rc != 0)
        goto error_exit;

    if (resp_code != HttpStatus_OK)
        goto error_exit;

    p = http_client_move_response_body(httpc, NULL);
    http_client_close(httpc);

    if (!p)
        return -1;

    *resp = p;
    return 0;

error_exit:
    http_client_close(httpc);
    return -1;
}

static int ipfs_file_stat(DStore *dstore, const char *uid,
                          const char *path, char **resp)
{
    char url[MAXPATHLEN + 1];
    http_client_t *httpc;
    long resp_code = 0;
    char *p;
    int rc;

    assert(resp);

    if (!has_working_node(dstore) && configure_working_node(dstore) < 0)
        return -1;

    rc = snprintf(url, sizeof(url), "http://%s:%u/api/v0/files/stat",
                  dstore->current_node_ip, (unsigned)dstore->current_node_port);
    if (rc < 0 || rc >= sizeof(url))
        return -1;

    httpc = http_client_new();
    if (!httpc)
        return -1;

    http_client_set_url(httpc, url);
    http_client_set_query(httpc, "uid", uid);
    http_client_set_query(httpc, "path", path);
    http_client_set_method(httpc, HTTP_METHOD_POST);
    http_client_set_request_body_instant(httpc, NULL, 0);
    http_client_enable_response_body(httpc);

    rc = http_client_request(httpc);
    if (rc != 0) {
        clear_working_node(dstore);
        goto error_exit;
    }

    rc = http_client_get_response_code(httpc, &resp_code);
    if (rc != 0)
        goto error_exit;

    if (resp_code != HttpStatus_OK)
        goto error_exit;

    p = http_client_move_response_body(httpc, NULL);
    http_client_close(httpc);

    if (!p)
        return -1;

    *resp = p;
    return 0;

error_exit:
    http_client_close(httpc);
    return -1;
}

static size_t read_response_body_cb(char *buffer,
                                    size_t size, size_t nitems, void *userdata)
{
    char *buf = (char *)(((void **)userdata)[0]);
    size_t bufsz = *((size_t *)(((void **)userdata)[1]));
    size_t *nrd = (size_t *)(((void **)userdata)[2]);
    size_t total_sz = size * nitems;

    if (*nrd + total_sz > bufsz)
        return 0;

    memcpy(buf + *nrd, buffer, total_sz);
    *nrd += total_sz;

    return total_sz;
}

static ssize_t ipfs_file_read(DStore *dstore, const char *uid,
                              const char *path, size_t offset,
                              void *buf, size_t len)
{
    char url[MAXPATHLEN + 1] = {0};
    char header[128];
    http_client_t *httpc;
    long resp_code = 0;
    size_t nrd = 0;
    void *user_data[] = {buf, &len, &nrd};
    int rc;

    if (!has_working_node(dstore) && configure_working_node(dstore) < 0)
        return -1;

    rc = snprintf(url, sizeof(url), "http://%s:%u/api/v0/files/read",
                  dstore->current_node_ip, (unsigned)dstore->current_node_port);
    if (rc < 0 || rc >= sizeof(url))
        return -1;

    httpc = http_client_new();
    if (!httpc)
        return -1;

    http_client_set_url(httpc, url);
    http_client_set_query(httpc, "uid", uid);
    http_client_set_query(httpc, "path", path);
    sprintf(header, "%zu", offset);
    http_client_set_query(httpc, "offset", header);
    sprintf(header, "%zu", len);
    http_client_set_query(httpc, "count", header);
    http_client_set_method(httpc, HTTP_METHOD_POST);
    http_client_set_request_body_instant(httpc, NULL, 0);
    http_client_set_response_body(httpc, read_response_body_cb, user_data);

    rc = http_client_request(httpc);
    if (rc != 0) {
        clear_working_node(dstore);
        goto error_exit;
    }

    rc = http_client_get_response_code(httpc, &resp_code);
    http_client_close(httpc);

    if (rc != 0)
        return -1;

    if (resp_code != HttpStatus_OK)
        return -1;

    return nrd;

error_exit:
    http_client_close(httpc);
    return -1;
}

static int ipfs_mkdir(DStore *dstore, const char *uid, const char *path)
{
    char url[MAXPATHLEN + 1] = {0};
    http_client_t *httpc;
    long resp_code;
    int rc;

    if (!has_working_node(dstore) && configure_working_node(dstore) < 0)
        return -1;

    rc = snprintf(url, sizeof(url), "http://%s:%u/api/v0/files/mkdir",
                  dstore->current_node_ip, (unsigned)dstore->current_node_port);
    if (rc < 0 || rc >= sizeof(url))
        return -1;

    httpc = http_client_new();
    if (!httpc)
        return -1;

    http_client_set_url(httpc, url);
    http_client_set_query(httpc, "uid", uid);
    http_client_set_query(httpc, "path", path);
    http_client_set_query(httpc, "parents", "true");
    http_client_set_method(httpc, HTTP_METHOD_POST);
    http_client_set_request_body_instant(httpc, NULL, 0);

    rc = http_client_request(httpc);
    if (rc != 0) {
        clear_working_node(dstore);
        goto error_exit;
    }

    rc = http_client_get_response_code(httpc, &resp_code);
    http_client_close(httpc);

    if (rc != 0)
        return -1;

    if (resp_code != HttpStatus_OK)
        return -1;

    return 0;

error_exit:
    http_client_close(httpc);
    return -1;
}

static int ipfs_publish(DStore *dstore, const char *uid, const char *hash)
{
    char url[MAXPATHLEN + 1] = {0};
    http_client_t *httpc;
    long resp_code = 0;
    int rc;

    if (!has_working_node(dstore) && configure_working_node(dstore) < 0)
        return -1;

    rc = snprintf(url, sizeof(url), "http://%s:%u/api/v0/name/publish",
                  dstore->current_node_ip, (unsigned)dstore->current_node_port);
    if (rc < 0 || rc >= sizeof(url))
        return -1;

    httpc = http_client_new();
    if (!httpc)
        return -1;

    http_client_set_url(httpc, url);
    http_client_set_query(httpc, "uid", uid);
    http_client_set_query(httpc, "path", hash);
    http_client_set_method(httpc, HTTP_METHOD_POST);
    http_client_set_request_body_instant(httpc, NULL, 0);

    rc = http_client_request(httpc);
    if (rc != 0) {
        clear_working_node(dstore);
        goto error_exit;
    }

    rc = http_client_get_response_code(httpc, &resp_code);
    http_client_close(httpc);

    if (rc != 0)
        return -1;

    if (resp_code != HttpStatus_OK)
        return -1;

    return 0;

error_exit:
    http_client_close(httpc);
    return -1;
}

static ssize_t ipfs_file_write(DStore *dstore, const char *uid,
                               const char *path, size_t offset,
                               const uint8_t *value, size_t len)
{
    char url[MAXPATHLEN + 1] = {0};
    char header[128] = {0};
    http_client_t *httpc;
    long resp_code = 0;
    int rc;

    if (!has_working_node(dstore) && configure_working_node(dstore) < 0)
        return -1;

    rc = snprintf(url, sizeof(url), "http://%s:%u/api/v0/files/write",
                  dstore->current_node_ip, (unsigned)dstore->current_node_port);
    if (rc < 0 || rc >= sizeof(url))
        return -1;

    httpc = http_client_new();
    if (!httpc)
        return -1;

    http_client_set_url(httpc, url);
    http_client_set_query(httpc, "uid", uid);
    http_client_set_query(httpc, "path", path);
    sprintf(header, "%zu", offset);
    http_client_set_query(httpc, "offset", header);
    sprintf(header, "%zu", len);
    http_client_set_query(httpc, "count", header);
    http_client_set_query(httpc, "create", "true");
    http_client_set_mime_instant(httpc, "file", NULL, NULL, (const char *)value, len);
    http_client_set_method(httpc, HTTP_METHOD_POST);

    rc = http_client_request(httpc);
    if (rc != 0) {
        clear_working_node(dstore);
        goto error_exit;
    }

    rc = http_client_get_response_code(httpc, &resp_code);
    http_client_close(httpc);

    if (rc != 0)
        return -1;

    if (resp_code != HttpStatus_OK)
        return -1;

    return 0;

error_exit:
    http_client_close(httpc);
    return -1;
}

static int ipfs_file_remove(DStore *dstore, const char *uid, const char *path)
{
    char url[MAXPATHLEN + 1] = {0};
    http_client_t *httpc;
    long resp_code = 0;
    int rc;

    if (!has_working_node(dstore) && configure_working_node(dstore) < 0)
        return -1;

    rc = snprintf(url, sizeof(url), "http://%s:%u/api/v0/files/rm",
                  dstore->current_node_ip, (unsigned)dstore->current_node_port);
    if (rc < 0 || rc >= sizeof(url))
        return -1;

    httpc = http_client_new();
    if (!httpc)
        return -1;

    http_client_set_url(httpc, url);
    http_client_set_query(httpc, "uid", uid);
    http_client_set_query(httpc, "path", path);
    http_client_set_query(httpc, "recursive", "true");
    http_client_set_method(httpc, HTTP_METHOD_POST);
    http_client_set_request_body_instant(httpc, NULL, 0);

    rc = http_client_request(httpc);
    if (rc) {
        clear_working_node(dstore);
        goto error_exit;
    }

    rc = http_client_get_response_code(httpc, &resp_code);
    http_client_close(httpc);

    if (rc != 0)
        return -1;

    if (resp_code != HttpStatus_OK)
        return -1;

    return 0;

error_exit:
    http_client_close(httpc);
    return -1;
}

static int choose_working_node(DStore *dstore)
{
    rpc_node_t *rpc_nodes = dstore->rpc_nodes;
    bool found = false;
    size_t base;
    size_t i;
    int rc;

    srand((unsigned)time(NULL));
    base = (size_t)rand() % dstore->rpc_nodes_count;
    i = base;

    do {
        rpc_node_t *rpc_node = rpc_nodes + i;

        if (*rpc_node->ipv4) {
            rc = ipfs_get_node_version(rpc_node->ipv4, rpc_node->port);
            if (!rc) {
                strcpy(dstore->current_node_ip, rpc_node->ipv4);
                dstore->current_node_port = rpc_node->port;
                found = true;
                break;
            }
        }

        if (*rpc_node->ipv6) {
            rc = ipfs_get_node_version(rpc_node->ipv6, rpc_node->port);
            if (!rc) {
                strcpy(dstore->current_node_ip, rpc_node->ipv6);
                dstore->current_node_port = rpc_node->port;
                found = true;
                break;
            }
        }

        i = (i + 1) % dstore->rpc_nodes_count;
    } while (i != base);

    return found ? 0 : -1;
}

static int login_working_node(DStore *dstore)
{
    char *resp;
    cJSON *json = NULL;
    cJSON *peer_id;
    cJSON *hash;
    int rc;

    rc = ipfs_get_uid_info(dstore, DSTORE_UID, &resp);
    if (rc < 0)
        return rc;

    json = cJSON_Parse(resp);
    free(resp);

    if (!json)
        return -1;

    peer_id = cJSON_GetObjectItemCaseSensitive(json, "PeerID");
    if (!cJSON_IsString(peer_id) || !peer_id->valuestring ||
        !*peer_id->valuestring) {
        cJSON_Delete(json);
        return -1;
    }

    rc = ipfs_resolve(dstore, peer_id->valuestring, &resp);
    cJSON_Delete(json);

    if (rc < 0)
        return rc;

    json = cJSON_Parse(resp);
    free(resp);

    if (!json)
        return -1;

    hash = cJSON_GetObjectItemCaseSensitive(json, "Path");
    if (!cJSON_IsString(hash) || !hash->valuestring || !*hash->valuestring) {
        cJSON_Delete(json);
        return -1;
    }

    rc = ipfs_login(dstore, DSTORE_UID, hash->valuestring);
    cJSON_Delete(json);

    if (rc < 0)
        return rc;

    return 0;
}

static int configure_working_node(DStore *dstore)
{
    int rc;

    rc = choose_working_node(dstore);
    if (rc < 0)
        return -1;

    rc = login_working_node(dstore);
    if (rc < 0)
        return -1;

    return 0;
}

DStore *dstore_create(RpcNode *rpc_nodes, size_t count)
{
    DStore *dstore;
    size_t i;
    int rc;

    dstore = rc_zalloc(sizeof(DStore) + sizeof(rpc_node_t) * count, NULL);
    if (!dstore)
        return NULL;

    dstore->rpc_nodes_count = count;

    for (i = 0; i < count ; ++i) {
        RpcNode  *from = rpc_nodes + i;
        rpc_node_t *to = dstore->rpc_nodes + i;

        if (from->ipv4)
            strcpy(to->ipv4, from->ipv4);
        if (from->ipv6)
            strcpy(to->ipv6, from->ipv6);

        to->port = from->port;
    }

    return dstore;
}

void dstore_destroy(DStore *dstore)
{
    deref(dstore);
}

static cJSON *parse_list_files_response(const char *response)
{
    cJSON *json;
    cJSON *entries;
    cJSON *entry;

    assert(response);

    json = cJSON_Parse(response);
    if (!json)
        return NULL;

    entries = cJSON_GetObjectItemCaseSensitive(json, "Entries");
    if (!entries || (!cJSON_IsArray(entries) && !cJSON_IsNull(entries))) {
        cJSON_Delete(json);
        return NULL;
    }

    if (cJSON_IsNull(entries))
        return json;

    cJSON_ArrayForEach(entry, entries) {
        cJSON *name;

        if (!cJSON_IsObject(entry)) {
            cJSON_Delete(json);
            return NULL;
        }

        name = cJSON_GetObjectItemCaseSensitive(entry, "Name");
        if (!name || !cJSON_IsString(name) || !name->valuestring ||
            !*name->valuestring) {
            cJSON_Delete(json);
            return NULL;
        }
    }

    return json;
}

static int read_file(DStore *dstore, const char *path, void **pdata, size_t *plen)
{
    int rc;
    cJSON *json;
    cJSON *size;
    char *resp;
    size_t len;
    void *data;
    ssize_t nrd;

    rc = ipfs_file_stat(dstore, DSTORE_UID, path, &resp);
    if (rc < 0)
        return -1;

    json = cJSON_Parse(resp);
    free(resp);

    if (!json)
        return -1;

    size = cJSON_GetObjectItem(json, "Size");
    if (!size || !cJSON_IsNumber(size)) {
        cJSON_Delete(json);
        return -1;
    }

    len = (size_t)size->valuedouble;
    cJSON_Delete(json);

    data = calloc(1, len);
    if (!data)
        return -1;

    nrd = ipfs_file_read(dstore, DSTORE_UID, path, 0, data, len);
    if (nrd < 0) {
        free(data);
        return -1;
    }

    *pdata = data;
    *plen = nrd;
    return 0;
}

int dstore_get_values(DStore *dstore, const char *key,
                      bool (*cb)(const char *key, const uint8_t *value,
                                 size_t length, void *ctx),
                      void *ctx)
{
    char path[MAXPATHLEN + 1] = {0};
    cJSON *resp_json;
    cJSON *entries;
    cJSON *entry;
    char *resp;
    int rc;

    sprintf(path, "%s/%s", MSG_PATH, key);
    rc = ipfs_list_files(dstore, DSTORE_UID, path, &resp);
    if (rc < 0)
        return -1;

    resp_json = parse_list_files_response(resp);
    free(resp);

    if (!resp_json)
        return -1;

    entries = cJSON_GetObjectItemCaseSensitive(resp_json, "Entries");
    if (cJSON_IsArray(entries)) {
        cJSON_ArrayForEach(entry, entries) {
            char fpath[MAXPATHLEN + 1];
            cJSON *name;
            void *data;
            size_t len;
            bool resume;

            name = cJSON_GetObjectItemCaseSensitive(entry, "Name");
            sprintf(fpath, "%s/%s", path, name->valuestring);

            rc = read_file(dstore, fpath, &data, &len);
            if (rc < 0) {
                cJSON_Delete(resp_json);
                return -1;
            }

            resume = cb(key, data, len, ctx);
            free(data);

            if (!resume) {
                cJSON_Delete(resp_json);
                return 0;
            }
        }
    }
    cJSON_Delete(resp_json);
    return 0;
}

static int get_root_hash(DStore *dstore, char *buf, size_t bufsz)
{
    int rc;
    char *resp;
    cJSON *json;
    cJSON *hash;

    rc = ipfs_file_stat(dstore, DSTORE_UID, "/", &resp);
    if (rc < 0)
        return -1;

    json = cJSON_Parse(resp);
    free(resp);

    if (!json)
        return -1;

    hash = cJSON_GetObjectItem(json, "Hash");
    if (!cJSON_IsString(hash) || !hash->valuestring || !*hash->valuestring) {
        cJSON_Delete(json);
        return -1;
    }

    rc = snprintf(buf, bufsz, "/ipfs/%s", hash->valuestring);
    cJSON_Delete(json);

    if (rc < 0 || rc >= bufsz)
        return -1;

    return 0;
}

static int publish_root_hash(DStore *dstore)
{
    char hash[1024] = {0};
    int rc;

    rc = get_root_hash(dstore, hash, sizeof(hash));
    if (rc < 0)
        return -1;

    rc = ipfs_publish(dstore, DSTORE_UID, hash);
    if (rc < 0)
        return -1;

    return 0;
}

int dstore_add_value(DStore *dstore, const char *key, const uint8_t *value, size_t len)
{
    char path[MAXPATHLEN + 1] = {0};
    int rc;
    ssize_t nwr;

    sprintf(path, "%s/%s", MSG_PATH, key);
    rc = ipfs_mkdir(dstore, DSTORE_UID, path);
    if (rc < 0)
        return -1;

    sprintf(path + strlen(path), "/%llu", (unsigned long long)time(NULL));
    nwr = ipfs_file_write(dstore, DSTORE_UID, path, 0, value, len);
    if (nwr < 0)
        return -1;

    rc = publish_root_hash(dstore);
    if (rc < 0)
        return -1;

    return 0;
}

int dstore_remove_values(DStore *dstore, const char *key)
{
    char path[MAXPATHLEN + 1] = {0};
    int rc;
    ssize_t nwr;

    sprintf(path, "%s/%s", MSG_PATH, key);
    rc = ipfs_file_remove(dstore, DSTORE_UID, path);
    if (rc < 0)
        return -1;

    rc = publish_root_hash(dstore);
    if (rc < 0)
        return -1;

    return 0;
}
