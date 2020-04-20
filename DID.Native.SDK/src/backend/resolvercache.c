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
#include <sys/stat.h>
#include <limits.h>
#include <assert.h>

#include "ela_did.h"
#include "diderror.h"
#include "common.h"
#include "did.h"
#include "resolvercache.h"

static char rootpath[PATH_MAX] = {0};

int ResolverCache_SetCacheDir(const char *root)
{
    int rc;

    assert(root && *root);

    if (strlen(root) >= sizeof(rootpath)) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    rc = mkdirs(root, S_IRWXU);
    if (rc < 0) {
        DIDError_Set(DIDERR_IO_ERROR, "Create cache directory (%s) failed", root);
        return rc;
    }

    strcpy(rootpath, root);
    return rc;
}

const char *ResolverCache_GetCacheDir(void)
{
    if (!*rootpath) {
        DIDError_Set(DIDERR_NOT_EXISTS, "No cache directory.");
        return NULL;
    }

    return rootpath;
}

int ResolverCache_Reset(void)
{
    if (!*rootpath)
        return 0;

    delete_file(rootpath);
    return 0;
}

int ResolverCache_Load(ResolveResult *result, DID *did, long ttl)
{
    char path[PATH_MAX];
    const char *data;
    struct stat s;
    time_t curtime;
    cJSON *root;
    int rc;

    assert(result);
    assert(did);
    assert(ttl >= 0);

    if (get_file(path, 0, 2, rootpath, did->idstring) == -1)
        return -1;

    //check the lasted modify time
    if (stat(path, &s) < 0)
        return -1;

    time(&curtime);
    if (curtime - s.st_mtime > ttl)
        return -1;

    data = load_file(path);
    if (!data)
        return -1;

    root = cJSON_Parse(data);
    free((char*)data);
    if (!root)
        return -1;

    rc = ResolveResult_FromJson(result, root, false);
    cJSON_Delete(root);
    return rc;
}

int ResolveCache_Store(ResolveResult *result, DID *did)
{
    char path[PATH_MAX];
    const char *data;
    int rc;

    assert(result);
    assert(did);
    
    if (get_file(path, 1, 2, rootpath, did->idstring) == -1) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Create resolver cache entry failed.");
        return -1;
    }

    data = ResolveResult_ToJson(result);
    if (!data) {
        DIDError_Set(DIDERR_MALFORMED_RESOLVE_RESULT, "Serialize the resolve result to json failed.");
        return -1;
    }

    rc = store_file(path, data);
    free((char*)data);
    if (rc < 0)
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Store resolver result data failed.");

    return rc;
}
