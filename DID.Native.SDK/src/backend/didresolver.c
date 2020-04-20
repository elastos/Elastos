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

#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <assert.h>

#include "ela_did.h"
#include "diderror.h"
#include "didresolver.h"

#define DID_RESOLVE_REQUEST "{\"method\":\"resolvedid\",\"params\":{\"did\":\"%s\",\"all\":%s}}"

typedef struct HttpResponseBody {
    size_t used;
    size_t sz;
    void *data;
} HttpResponseBody;

static size_t HttpResponseBodyWriteCallback(char *ptr,
        size_t size, size_t nmemb, void *userdata)
{
    HttpResponseBody *response = (HttpResponseBody *)userdata;
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

        new_sz += 16;

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

typedef struct HttpRequestBody {
    size_t used;
    size_t sz;
    char *data;
} HttpRequestBody;

static size_t HttpRequestBodyReadCallback(void *dest, size_t size,
        size_t nmemb, void *userdata)
{
    HttpRequestBody *request = (HttpRequestBody *)userdata;
    size_t length = size * nmemb;
    size_t bytes_copy = request->sz - request->used;

    if (bytes_copy) {
        if(bytes_copy > length)
            bytes_copy = length;

        memcpy(dest, request->data + request->used, bytes_copy);

        request->used += bytes_copy;
        return bytes_copy;
    }

    return 0;
}

// Caller need free the pointer
static const char *DefaultResolver_Resolve(DIDResolver *resolver, const char *did, int all)
{
    DefaultResolver *_resolver = (DefaultResolver *)resolver;

    HttpRequestBody request;
    HttpResponseBody response;
    char buffer[256];
    const char *forAll;

    if (!resolver || !*_resolver->url || !did) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    // TODO: max did length
    if (strlen(did) > 64) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments: too long did string.");
        return NULL;
    }

    forAll = !all ? "false" : "true";

    request.used = 0;
    request.sz = sprintf(buffer, DID_RESOLVE_REQUEST, did, forAll);
    request.data = buffer;

    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, _resolver->url);

    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, HttpRequestBodyReadCallback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &request);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)request.sz);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HttpResponseBodyWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    memset(&response, 0, sizeof(response));
    CURLcode rc = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    if (rc != CURLE_OK) {
        DIDError_Set(DIDERR_RESOLVE_ERROR, "Resolve [%s] error, status: %d, message: %s", did, rc, curl_easy_strerror(rc));
        if (response.data)
            free(response.data);

        return NULL;
    }

    ((char *)response.data)[response.used] = 0;
    return (const char *)response.data;
}

DIDResolver *DefaultResolver_Create(const char *url)
{
    CURLcode rc = curl_global_init(CURL_GLOBAL_ALL);
    if (rc != CURLE_OK) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    DefaultResolver *resolver = (DefaultResolver*)calloc(1, sizeof(DefaultResolver));
    if (!resolver) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Malloc buffer for Default Resolver failed.");
        return NULL;
    }

    memcpy((char*)resolver->url, url, strlen(url) + 1);
    resolver->base.resolve = DefaultResolver_Resolve;
    return (DIDResolver *)resolver;
}

void DefaultResolver_Destroy(DIDResolver *resolver)
{
    if (resolver)
        free(resolver);

    curl_global_cleanup();
}



