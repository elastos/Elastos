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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_IO_H
#include <io.h>
#endif
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#ifdef HAVE_GLOB_H
#include <glob.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdarg.h>
#include <fnmatch.h>
#include <pthread.h>
#include <openssl/opensslv.h>
#include <cjson/cJSON.h>

#include "ela_did.h"
#include "didstore.h"
#include "common.h"
#include "credential.h"
#include "diddocument.h"
#include "crypto.h"
#include "HDkey.h"
#include "didbackend.h"
#include "credential.h"
#include "didmeta.h"
#include "credmeta.h"

DIDStore *storeInstance = NULL;

static char *StoreTag = "/.meta";
static char MAGIC[] = { 0x00, 0x0D, 0x01, 0x0D };
static char VERSION[] = { 0x00, 0x00, 0x00, 0x01 };
static const char *ProofType = "ECDSAsecp256r1";

typedef enum DIDStore_Type {
    DIDStore_Root,
    DIDStore_Rootkey,
    DIDStore_RootIndex,
    DIDStore_RootMnemonic,
    DIDStore_DID,
    DIDStore_DIDMeta,
    DIDStore_Doc,
    DIDStore_CredentialRoot,
    DIDStore_Credential,
    DIDStore_CredentialMeta,
    DIDStore_PrivateKey
} DIDStore_Type;

static const char* methods[] = { "", "key", "index", "", "mnemonic", "", "document",
            "credentials", "credentials", "credentials", "privatekeys"};

typedef struct DID_Alias_Helper {
    DIDStore *store;
    DIDStore_GetDIDAliasCallback *cb;
} DID_Alias_Helper;

typedef struct Cred_Alias_Helper {
    DIDStore *store;
    DIDStore_GetCredAliasCallback *cb;
    DID did;
    const char *type;
} Cred_Alias_Helper;

static int test_path(const char *path)
{
    struct stat s;

    assert(path);

    if (stat(path, &s) < 0)
        return -1;

    if (s.st_mode & S_IFDIR)
        return S_IFDIR;
    else if (s.st_mode & S_IFREG)
        return S_IFREG;
    else
        return -1;
}

static int list_dir(const char *path, const char *pattern,
        int (*callback)(const char *name, void *context), void *context)
{
    char full_pattern[PATH_MAX];
    size_t len;

    assert(path);
    assert(pattern);

    len = snprintf(full_pattern, sizeof(full_pattern), "%s/%s", path, pattern);
    if (len == sizeof(full_pattern))
        full_pattern[len-1] = 0;

#if defined(_WIN32) || defined(_WIN64)
    struct _finddata_t c_file;
    intptr_t hFile;

    if ((hFile = _findfirst(full_pattern, &c_file )) == -1L)
        return -1;

    do {
        if(callback(c_file.name, context) < 0)
            break;
    } while (_findnext(hFile, &c_file) == 0);

    _findclose(hFile);
#else
    glob_t gl;
    size_t pos = strlen(path) + 1;

    memset(&gl, 0, sizeof(gl));

    glob(full_pattern, GLOB_DOOFFS, NULL, &gl);

    for (int i = 0; i < gl.gl_pathc; i++) {
        char *fn = gl.gl_pathv[i] + pos;
        if(callback(fn, context) < 0)
            break;
    }

    globfree(&gl);
#endif

    return 0;
}

static void delete_file(const char *path);

static int delete_file_helper(const char *path, void *context)
{
    char fullpath[PATH_MAX];
    int len;

    if (!path)
        return 0;

    if (strcmp(path, ".") != 0 && strcmp(path, "..") != 0) {
        len = snprintf(fullpath, sizeof(fullpath), "%s/%s", (char *)context, path);
        if (len < 0 || len > PATH_MAX)
            return -1;

        delete_file(fullpath);
    }

    return 0;
}

static void delete_file(const char *path)
{
    int rc;

    assert(path);

    rc = test_path(path);
    if (rc < 0)
        return;
    else if (rc == S_IFDIR) {
        list_dir(path, ".*", delete_file_helper, (void *)path);

        if (list_dir(path, "*", delete_file_helper, (void *)path) == 0)
            rmdir(path);
    } else
        remove(path);
}

static const char *get_file_root(DIDStore *store, DIDStore_Type type,
        const char *didstring, const char *fragment, int create)
{
    static __thread char file_root[PATH_MAX];
    int len;
    char file_suffix[PATH_MAX];

    if (type > DIDStore_PrivateKey || (type > DIDStore_DID && !didstring))
        return NULL;

    strcpy(file_root, store->root);
    if (create && test_path(file_root) < 0 && mkdir(file_root, S_IRWXU) == -1)
        return NULL;

    if (type == DIDStore_Root)
        return file_root;

    if (type != DIDStore_Rootkey && type != DIDStore_RootIndex) {
        len = snprintf(file_root, sizeof(file_root), "%s/ids", store->root);
        if (len < 0 || len > sizeof(file_root))
            return NULL;

        if (create && test_path(file_root) < 0 && mkdir(file_root, S_IRWXU) == -1)
            return NULL;
    }

    if (type == DIDStore_DID)
        return file_root;

    len = snprintf(file_suffix, sizeof(file_suffix), "/%s", didstring);
    if (len < 0 || len > sizeof(file_suffix))
        return NULL;

    strcat(file_root, file_suffix);

    if (create && test_path(file_root) < 0 && mkdir(file_root, S_IRWXU) == -1)
        return NULL;

    if (type == DIDStore_Doc || type == DIDStore_Rootkey ||
            type == DIDStore_RootIndex || type == DIDStore_DIDMeta)
       return file_root;

    len = snprintf(file_suffix, sizeof(file_suffix), "/%s", methods[type]);
    if (len < 0 || len > sizeof(file_suffix))
        return NULL;

    strcat(file_root, file_suffix);

    if (create && test_path(file_root) < 0 && mkdir(file_root, S_IRWXU) == -1)
        return NULL;

    if (type == DIDStore_CredentialRoot)
        return file_root;

    if (!fragment)
        return NULL;

    len = snprintf(file_suffix, sizeof(file_suffix), "/%s", fragment);
    if (len < 0 || len > sizeof(file_suffix))
        return NULL;

    strcat(file_root, file_suffix);
    if (type == DIDStore_PrivateKey)
        return file_root;

    if (create && test_path(file_root) < 0 && mkdir(file_root, S_IRWXU) == -1)
        return NULL;

    return file_root;
}

static const char *get_file_path(DIDStore *store, DIDStore_Type type,
        const char *didstring, const char *fragment, int create)
{
    static __thread char file_path[PATH_MAX];
    int len;
    const char *root;

    if (type > DIDStore_PrivateKey || (type > DIDStore_Root && !didstring))
        return NULL;

    root = get_file_root(store, type, didstring, fragment, create);
    if (!root)
        return NULL;

    if (type == DIDStore_Root || type == DIDStore_PrivateKey
            || type == DIDStore_CredentialRoot)
        return root;

    if (type == DIDStore_DID)
        return get_file_root(store, DIDStore_Doc, didstring, fragment, create);

    if (type == DIDStore_Doc || type == DIDStore_Rootkey || type == DIDStore_RootIndex) {
        len = snprintf(file_path, sizeof(file_path), "%s/%s", root, methods[type]);
        if (len < 0 || len > sizeof(file_path))
            return NULL;

        return file_path;
    }

    if (type == DIDStore_DIDMeta || type == DIDStore_CredentialMeta) {
        len = snprintf(file_path, sizeof(file_path), "%s/.meta", root);
        if (len < 0 || len > sizeof(file_path))
            return NULL;

        return file_path;
    }

    len = snprintf(file_path, sizeof(file_path), "%s/%s", root, "credential");
    if (len < 0 || len > sizeof(file_path))
        return NULL;

    return file_path;
}

static int store_file(const char *path, const char *string)
{
    int fd;
    size_t string_len, size;

    assert(path);
    assert(string);

    fd = open(path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1)
        return -1;

    string_len = strlen(string);
    size = write(fd, string, string_len);
    if (size < string_len) {
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

static int load_files(const char *path, const char **string)
{
    int fd;
    size_t size;
    struct stat st;

    if (!path || !string)
        return -1;

    fd = open(path, O_RDONLY);
    if (fd == -1)
        return -1;

    if (fstat(fd, &st) < 0) {
        close(fd);
        return -1;
    }

    size = st.st_size;
    *string = (const char*)calloc(1, size + 1);
    if (!*string) {
        close(fd);
        return -1;
    }

    if (read(fd, (char*)*string, size) != size) {
        free((char*)(*string));
        *string = NULL;
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

static int is_empty_helper(const char *path, void *context)
{
    *(int *)context = 1;
    return -1;
}

static bool is_empty(const char *path)
{
    int flag = 0;

    assert(path);

    if (list_dir(path, "*", is_empty_helper, &flag) < 0)
        return false;

    if (flag)
        return true;

    return false;
}

static int store_didmeta(DIDStore *store, DIDMeta *meta, DID *did)
{
    const char *path, *data;
    int rc;

    assert(store);
    assert(meta);
    assert(did);

    path = get_file_path(store, DIDStore_DIDMeta, did->idstring, NULL, 1);
    if (!path)
        return -1;

    if (test_path(path) == S_IFDIR)
        delete_file(path);

    data = DIDMeta_ToJson(meta);
    rc = store_file(path, data);
    free((char*)data);
    return rc;
}

static int load_didmeta(DIDStore *store, DIDMeta *meta, const char *did)
{
    const char *data, *path;
    int rc;

    assert(store);
    assert(meta);
    assert(did);

    path = get_file_path(store, DIDStore_DIDMeta, did, NULL, 0);
    if (!path)
        return -1;

    rc = test_path(path);
    if (rc < 0)
        return -1;
    else if (rc == S_IFDIR) {
        delete_file(path);
        return -1;
    }

    if (load_files(path, &data) == -1)
        return -1;
    rc = DIDMeta_FromJson(meta, data);
    free((char*)data);
    return rc;
}

int didstore_loaddidmeta(DIDStore *store, DIDMeta *meta, DID *did)
{
    bool iscontain;

    if (!store || !meta || !did)
        return -1;

    iscontain = DIDStore_ContainsDID(store, did);
    if (!iscontain)
        return -1;

    return load_didmeta(store, meta, did->idstring);
}

int didstore_storedidmeta(DIDStore *store, DIDMeta *meta, DID *did)
{
    bool iscontain;

    if (!store || !did || !meta)
        return -1;

    iscontain = DIDStore_ContainsDID(store, did);
    if (!iscontain)
        return -1;

    return store_didmeta(store, meta, did);
}

static int store_credmeta(DIDStore *store, CredentialMeta *meta, DIDURL *id)
{
    const char *path, *data;
    bool iscontain;
    int rc;

    assert(store);
    assert(meta);
    assert(id);

    path = get_file_path(store, DIDStore_CredentialMeta, id->did.idstring, id->fragment, 1);
    if (!path)
        return -1;

    if (test_path(path) == S_IFDIR)
        delete_file(path);

    data = CredentialMeta_ToJson(meta);
    rc = store_file(path, data);
    free((char*)data);
    return rc;
}

static int load_credmeta(DIDStore *store, CredentialMeta *meta, const char *did,
        const char *fragment)
{
    const char *data, *path;
    int rc;

    assert(store);
    assert(meta);
    assert(did);
    assert(fragment);

    path = get_file_path(store, DIDStore_CredentialMeta, did, fragment, 0);
    if (!path)
        return -1;

    rc = test_path(path);
    if (rc < 0)
        return -1;
    else if (rc == S_IFDIR) {
        delete_file(path);
        return -1;
    }

    if (load_files(path, &data) == -1)
        return -1;

    rc = CredentialMeta_FromJson(meta, data);
    free((char*)data);
    return rc;
}

int didstore_storecredmeta(DIDStore *store, CredentialMeta *meta, DIDURL *id)
{
    bool iscontain;

    if (!store || !meta || !id)
        return -1;

    iscontain = DIDStore_ContainsCredential(store, DIDURL_GetDid(id), id);
    if (!iscontain)
        return -1;

    return store_credmeta(store, meta, id);
}

int didstore_loadcredmeta(DIDStore *store, CredentialMeta *meta, DIDURL *id)
{
    bool iscontain;

    if (!store || !meta || !id)
        return -1;

    iscontain = DIDStore_ContainsCredential(store, DIDURL_GetDid(id), id);
    if (!iscontain)
        return -1;

    return load_credmeta(store, meta, id->did.idstring, id->fragment);
}

static int create_store(DIDStore *store)
{
    int fd;
    size_t size;
    char TagFile[PATH_MAX];

    assert(store);

    if (!get_file_path(store, DIDStore_Root, NULL, NULL, 1))
        return -1;

    strcpy(TagFile, store->root);
    strcat(TagFile, StoreTag);

    fd = open(TagFile, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1)
        return -1;

    size = write(fd, MAGIC, sizeof(MAGIC));
    if (size < sizeof(MAGIC)) {
        close(fd);
        return -1;
    }

    size = write(fd, VERSION, sizeof(VERSION));
    if (size < sizeof(VERSION)) {
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

static int check_store(DIDStore *store)
{
    int fd;
    char symbol[1];
    int i, flag = 0;
    size_t size;
    char TagFile[PATH_MAX];

    assert(store);

    if (test_path(store->root) != S_IFDIR)
        return -1;

    strcpy(TagFile, store->root);
    strcat(TagFile, StoreTag);

    fd = open(TagFile, O_RDONLY);
    if (fd == -1)
        return -1;

    while (read(fd, symbol, sizeof(char)) == 1) {
        for (i = 0; i < sizeof(MAGIC); i++) {
            if (symbol[0] == MAGIC[i])
                flag = 1;
        }
        if (!flag) {
            close(fd);
            return -1;
        }
    }
    flag = 0;
    while (read(fd, symbol, sizeof(char)) == 1) {
        for (i = 0; i < sizeof(VERSION); i++) {
            if (symbol[0] == VERSION[i])
                flag = 1;
        }
        if (!flag) {
            close(fd);
            return -1;
        }
    }

    close(fd);
    return 0;
}

static int store_seed(DIDStore *store, uint8_t *seed, size_t size, const char *storepass)
{
    unsigned char base64[512];

    assert(seed);
    assert(storepass);
    assert(*storepass);

    if (encrypt_to_base64((char *)base64, storepass, seed, size) == -1)
        return -1;

    if (store_file(get_file_path(store, DIDStore_Rootkey, "private", NULL, 1),
            (const char *)base64) == -1)
        return -1;

    return 0;
}

static ssize_t load_seed(DIDStore *store, uint8_t *seed, size_t size, const char *storepass)
{
    const char *encrpted_seed;
    unsigned char binseed[SEED_BYTES];
    ssize_t len;

    assert(store);
    assert(seed);
    assert(storepass);
    assert(*storepass);
    assert(size > SEED_BYTES);

    if (load_files(get_file_path(store, DIDStore_Rootkey, "private", NULL, 0),
            &encrpted_seed) == -1)
        return -1;

    len = decrypt_from_base64(binseed, storepass, encrpted_seed);
    free((char*)encrpted_seed);
    memcpy(seed, binseed, size);

    return len;
}

static int get_last_index(DIDStore *store)
{
    int index;
    const char *index_string;

    assert(store);

    if (load_files(get_file_path(store, DIDStore_RootIndex, "private", NULL, 0), &index_string) == -1
        || !index_string)
        index = 0;
    else {
        index = atoi(index_string);
        free((char*)index_string);
    }

    return index;
}

static int store_index(DIDStore *store, int index)
{
    char string[32];
    int len;

    assert(store);
    assert(index >= 0);

    len = snprintf(string, sizeof(string), "%d", index);
    if (len < 0 || len > sizeof(string))
        return -1;

    if (store_file(get_file_path(store, DIDStore_RootIndex, "private", NULL, 1),
                string) == -1)
        return -1;

    return 0;
}

static int list_did_helper(const char *path, void *context)
{
    DID_Alias_Helper *dh = (DID_Alias_Helper*)context;
    DIDEntry *entry;
    DIDMeta meta;
    int rc;

    if (strcmp(path, ".") == 0 || strcmp(path, "..") == 0)
        return 0;

    entry = (DIDEntry*)calloc(1, sizeof(DIDEntry));
    if (!entry)
        return -1;

    strcpy((char*)entry->did.idstring, path);
    if (load_didmeta(dh->store, &meta, path) == -1)
        return -1;

    rc = DIDMeta_GetAlias(&meta, entry->alias, sizeof(entry->alias));
    if (rc)
        return -1;

    dh->cb(entry, NULL);
    return 0;
}

static bool has_type(DID *did, const char *path, const char *type)
{
    const char *string;
    Credential *credential;
    int i;

    assert(did);
    assert(path);
    assert(type);

    if (load_files(path, &string) == -1)
        return false;

    credential = Credential_FromJson(string, did);
    free((char*)string);
    if (!credential)
        return false;

    for (i = 0; i < credential->type.size; i++) {
        const char *new_type = credential->type.types[i];
        if (!new_type)
            continue;
        if (strcmp(new_type, type) == 0) {
            Credential_Destroy(credential);
            return true;
        }
    }

    Credential_Destroy(credential);
    return false;
}

static int select_credential_helper(const char *path, void *context)
{
    const char* string;
    Credential *credential;
    CredentialEntry *entry;
    CredentialMeta meta;
    int rc;

    Cred_Alias_Helper *ch = (Cred_Alias_Helper*)context;

    assert(path);

    if (strcmp(path, ".") == 0 || strcmp(path, "..") == 0)
        return -1;

    if (load_files(path, &string) == -1)
        return -1;

    credential = Credential_FromJson(string, &(ch->did));
    free((char*)string);
    if (!credential)
        return -1;

    for (int j = 0; j < credential->type.size; j++) {
        const char *new_type = credential->type.types[j];
        if (!new_type)
            continue;
        if (strcmp(new_type, ch->type) == 0) {
            entry = (CredentialEntry*)calloc(1, sizeof(CredentialEntry));
            if (!entry)
                continue;

            strcpy((char*)entry->id.did.idstring, ch->did.idstring);
            strcpy((char*)entry->id.fragment, path);
            if (load_credmeta(ch->store, &meta, ch->did.idstring, path) == -1)
                continue;
            rc = CredentialMeta_GetAlias(&meta, entry->alias, sizeof(entry->alias));
            if (rc)
                continue;

            return ch->cb(entry, NULL);
        }
    }
    return -1;
}

static int list_credential_helper(const char *path, void *context)
{
    Cred_Alias_Helper *ch = (Cred_Alias_Helper*)context;
    CredentialEntry *entry;
    DIDURL *id;
    CredentialMeta meta;
    int rc;

    assert(path);

    if (strstr(path, ".") == 0 || strstr(path, "..") == 0)
        return 0;

    entry = (CredentialEntry*)calloc(1, sizeof(CredentialEntry));
    if (!entry)
        return -1;

    strcpy((char*)entry->id.did.idstring, ch->did.idstring);
    strcpy((char*)entry->id.fragment, path);
    if (load_credmeta(ch->store, &meta, ch->did.idstring, path) == -1)
        return -1;
    rc = CredentialMeta_GetAlias(&meta, entry->alias, sizeof(entry->alias));
    if (rc)
        return -1;

    return ch->cb(entry, NULL);
}

static DIDDocument *create_document(DID *did, const char *key,
        const char *storepass, const char *alias)
{
    DIDDocument *document;
    DIDURL id;
    DID controller;
    int rc;
    DIDDocumentBuilder *builder;

    assert(did);
    assert(key);
    assert(*key);
    assert(storepass);
    assert(*storepass);

    strcpy((char*)controller.idstring, did->idstring);
    strcpy((char*)id.did.idstring, did->idstring);
    strcpy((char*)id.fragment, "primary");

    builder = DIDDocument_Modify(NULL);
    if (!builder)
        return NULL;

    if (DID_Copy(&builder->document->did, did) == -1) {
        DIDDocumentBuilder_Destroy(builder);
        return NULL;
    }

    if (DIDDocumentBuilder_AddPublicKey(builder, &id, &controller, key) == -1) {
        DIDDocumentBuilder_Destroy(builder);
        return NULL;
    }

    if (DIDDocumentBuilder_AddAuthenticationKey(builder, &id, key) == -1) {
        DIDDocumentBuilder_Destroy(builder);
        return NULL;
    }

    DIDDocument_SetExpires(builder->document, 0);
    document = DIDDocumentBuilder_Seal(builder, storepass);
    if (!document) {
        DIDDocumentBuilder_Destroy(builder);
        return NULL;
    }

    if (DIDMeta_Init(&document->meta, alias, NULL, false, 0) == -1) {
        DIDDocument_Destroy(document);
        return NULL;
    }

    return document;
}

/////////////////////////////////////////////////////////////////////////
DIDStore* DIDStore_Initialize(const char *root, DIDAdapter *adapter)
{
    if (!root || !*root || !adapter)
        return NULL;

    DIDStore_Deinitialize();

    storeInstance = (DIDStore *)calloc(1, sizeof(DIDStore));
    if (!storeInstance)
        return NULL;

    strcpy(storeInstance->root, root);
    storeInstance->backend.adapter = adapter;

    if (get_file_path(storeInstance, DIDStore_Root, NULL, NULL, 0) && !check_store(storeInstance))
        return storeInstance;

    if (get_file_path(storeInstance, DIDStore_Root, NULL, NULL, 1) && !create_store(storeInstance))
        return storeInstance;

    free(storeInstance);
    return NULL;
}

DIDStore* DIDStore_GetInstance(void)
{
    if (!storeInstance)
        return NULL;

    return storeInstance;
}

void DIDStore_Deinitialize()
{
    if (!storeInstance)
        return;

    free(storeInstance);
    storeInstance = NULL;
}

int DIDStore_StoreDID(DIDStore *store, DIDDocument *document, const char *alias)
{
    const char *path, *string;
    DIDMeta *meta;

    if (!store || !document)
        return -1;

    path = get_file_path(store, DIDStore_Doc, document->did.idstring, NULL, 1);
    if (!path)
        return -1;

	string = DIDDocument_ToJson(document, 0, 0);
	if (!string)
		return -1;

    if (store_file(path, string) == 0) {
        free((char*)string);
        meta = document_getmeta(document);
        if (DIDMeta_SetAlias(meta, alias) == -1)
            return -1;

        if (store_didmeta(store, meta, &document->did) == 0)
            return 0;
		else {
			delete_file(path);
			const char *did_path = get_file_root(store, DIDStore_DID, document->did.idstring, NULL, 0);
			if (is_empty(did_path) == false)
				delete_file(did_path);
		}
    }
    return -1;
}

DIDDocument *DIDStore_LoadDID(DIDStore *store, DID *did)
{
    DIDDocument *document;
    const char *string, *path;
    int rc;

    if (!store || !did)
        return NULL;

    path = get_file_path(store, DIDStore_Doc, did->idstring, NULL, 0);
    rc = test_path(path);
    if (rc < 0)
        return NULL;
    else if (rc == S_IFDIR) {
        delete_file(path);
        return NULL;
    }

    if (load_files(path, &string) == -1)
        return NULL;

    document = DIDDocument_FromJson(string);
    free((char*)string);

    if (load_didmeta(store, &document->meta, document->did.idstring) == -1) {
        DIDDocument_Destroy(document);
        return NULL;
    }
    return document;
}

bool DIDStore_ContainsDID(DIDStore *store, DID *did)
{
    const char* path;
    int rc;

    if (!store || !did)
        return false;

    path = get_file_path(store, DIDStore_DID, did->idstring, NULL, 0);
    rc = test_path(path);
    if (rc < 0)
        return false;
    else if (rc == S_IFREG || is_empty(path) == false) {
        delete_file(path);
        return false;
    }

    return true;
}

void DIDStore_DeleteDID(DIDStore *store, DID *did)
{
    const char *root, *path;

    if (!store || !did)
        return;

    path = get_file_path(store, DIDStore_DID, did->idstring, NULL, 0);
    if (test_path(path) > 0)
        delete_file(path);

    path = get_file_path(store, DIDStore_DIDMeta, did->idstring, NULL, 0);
    if (test_path(path) > 0)
        delete_file(path);

    return;
}

int DIDStore_ListDID(DIDStore *store, DIDStore_GetDIDAliasCallback *callback,
        void *context)
{
    ssize_t size = 0;
    char dir_path[PATH_MAX];
    DID_Alias_Helper dh;
    int rc;

    if (!store || !callback)
        return -1;

    strcpy(dir_path, store->root);
    strcat(dir_path, "/ids");
    rc = test_path(dir_path);
    if (rc < 0)
        return -1;
    if (rc != S_IFDIR)
        return -1;

    dh.store = store;
    dh.cb = callback;

    if (list_dir(dir_path, "i*", list_did_helper, (void*)&dh) == -1)
        return -1;

    return 0;
}

int DIDStore_StoreCredential(DIDStore *store, Credential *credential, const char *alias)
{
    const char *path, *data;
    CredentialMeta *meta;

    if (!store || !credential)
        return -1;

    path = get_file_path(store, DIDStore_Credential, credential->id.did.idstring,
            credential->id.fragment, 1);
    if (!path)
        return -1;

    data = Credential_ToJson(credential, 1, 0);
    if (!data)
        return -1;

    if (store_file(path, data) == 0) {
        free((char*)data);
        meta = credential_getmeta(credential);
        if (CredentialMeta_SetAlias(meta, alias) == -1)
            return -1;
        if(store_credmeta(store, meta, &credential->id) == 0)
            return 0;
        else
            delete_file(path);
    }

    return -1;
}

Credential *DIDStore_LoadCredential(DIDStore *store, DID *did, DIDURL *id)
{
    const char *string, *path;
    Credential *credential;
    int rc;

    if (!store || !did ||!id)
        return NULL;

    path = get_file_path(store, DIDStore_Credential, did->idstring, id->fragment, 0);
    rc = test_path(path);
    if (rc < 0)
        return NULL;
    else if (rc == S_IFDIR) {
        delete_file(path);
        return NULL;
    }

    if (load_files(path, &string) == -1)
        return NULL;

    credential = Credential_FromJson(string, did);
    free((char*)string);
    return credential;
}

bool DIDStore_ContainsCredentials(DIDStore *store, DID *did)
{
    const char *path;
    int rc;

    if (!store || !did)
        return false;

    path = get_file_root(store, DIDStore_CredentialRoot, did->idstring, NULL, 0);
    rc = test_path(path);
    if (rc < 0)
        return false;
    else if (rc == S_IFREG) {
        delete_file(path);
        return false;
    }

    return is_empty(path);
}

bool DIDStore_ContainsCredential(DIDStore *store, DID *did, DIDURL *id)
{
    const char *path;
    int rc;

    if (!store || !did || !id || !DIDStore_ContainsCredentials(store, did))
        return false;

    path = get_file_path(store, DIDStore_Credential, did->idstring, id->fragment, 0);
    rc = test_path(path);
    if (rc < 0)
        return false;
    else if (rc == S_IFDIR) {
        delete_file(path);
        return false;
    }

    return true;
}

void DIDStore_DeleteCredential(DIDStore *store, DID *did, DIDURL *id)
{
    const char *root, *path, *meta_path;

    if (!store || !did || !id)
        return;

    path = get_file_path(store, DIDStore_Credential, did->idstring, id->fragment, 0);
    if (test_path(path) > 0)
        delete_file(path);

    meta_path = get_file_path(store, DIDStore_CredentialMeta, did->idstring, id->fragment, 0);
    if (test_path(meta_path) > 0)
        delete_file(meta_path);

    root = get_file_root(store, DIDStore_Credential, did->idstring, id->fragment, 0);
    if (root && !is_empty(root))
        delete_file(root);

    return;
}

int DIDStore_ListCredentials(DIDStore *store, DID *did,
        DIDStore_GetCredAliasCallback *callback, void *context)
{
    ssize_t size = 0;
    const char *dir_path;
    Cred_Alias_Helper ch;
    int rc;

    if (!store || !did || !callback)
        return -1;

    dir_path = get_file_root(store, DIDStore_CredentialRoot, did->idstring, NULL, 0);
    rc = test_path(dir_path);
    if (rc < 0)
        return -1;
    if (rc == S_IFREG) {
        delete_file(dir_path);
        return -1;
    }

    ch.store = store;
    ch.cb = callback;
    strcpy((char*)ch.did.idstring, did->idstring);
    ch.type = NULL;

    if (list_dir(dir_path, "*.*", list_credential_helper, (void*)&ch) == -1)
        return -1;

    return 0;
}

int DIDStore_SelectCredentials(DIDStore *store, DID *did, DIDURL *id,
        const char *type, DIDStore_GetCredAliasCallback *callback, void *context)
{
    const char *path = NULL;
    CredentialEntry entry;
    Cred_Alias_Helper ch;
    CredentialMeta meta;
    int rc;

    if (!store || !did || (!id && !type) || !callback)
        return -1;

    if (id) {
        path = get_file_path(store, DIDStore_Credential, did->idstring, id->fragment, 0);
        if (test_path(path) > 0) {
            if ((type && has_type(did, path, type) == true) || !type) {
                if (!load_credmeta(store, &meta, did->idstring, id->fragment))
                    CredentialMeta_GetAlias(&meta, entry.alias, sizeof(entry.alias));
                strcpy(entry.id.did.idstring, id->did.idstring);
                strcpy(entry.id.fragment, id->fragment);
                callback(&entry, NULL);
                return 0;
            }
            return -1;
        }
        return -1;
    }

    path = get_file_root(store, DIDStore_Credential, did->idstring, NULL, 0);

    rc = test_path(path);
    if (rc < 0)
        return -1;
    else if (rc == S_IFREG) {
        delete_file(path);
        return -1;
    }

    ch.store = store;
    ch.cb = callback;
    strcpy((char*)ch.did.idstring, did->idstring);
    ch.type = type;

    if (list_dir(path, "*.*", select_credential_helper, (void*)&ch) == -1)
        return -1;

    return 0;
}

bool DIDSotre_ContainPrivateKeys(DIDStore *store, DID *did)
{
    const char *dir_path;

    if (!store || !did)
        return false;

    dir_path = get_file_root(store, DIDStore_PrivateKey, did->idstring, NULL, 0);

    return is_empty(dir_path);
}

bool DIDStore_ContainPrivateKey(DIDStore *store, DID *did, DIDURL *id)
{
    const char *path;
    int rc;

    if (!store || !did || !id)
        return false;

    path = get_file_path(store, DIDStore_PrivateKey, did->idstring, id->fragment, 0);
    rc = test_path(path);
    if (rc < 0)
        return false;
    else if (rc == S_IFDIR) {
        delete_file(path);
        return false;
    }

    return true;
}

int DIDStore_StorePrivateKey(DIDStore *store, DID *did, DIDURL *id,
        const char *privatekey)
{
    size_t len;
    const char *path, *fragment;

    if (!store || !did || !id || !privatekey)
        return -1;

    if (!DID_Equals(DIDURL_GetDid(id), did))
        return -1;

    fragment = DIDURL_GetFragment(id);
    path = get_file_path(store, DIDStore_PrivateKey, did->idstring, fragment, 1);
    if (!path)
        return -1;

    if (store_file(path, privatekey) == -1)
        return -1;

    return 0;
}

void DIDStore_DeletePrivateKey(DIDStore *store, DID *did, DIDURL *id)
{
    const char *path;

    if (!store || !did || !id)
        return;

    path = get_file_path(store, DIDStore_PrivateKey, did->idstring, id->fragment, 0);
    if (test_path(path) > 0)
        delete_file(path);

    return;
}

static int refresh_did_fromchain(DIDStore *store, const char *storepass,
        uint8_t *seed, DID *did, uint8_t *publickey, size_t size)
{
    int index, last_index;
    int rc;
    DIDDocument *document;
    uint8_t last_publickey[PUBLICKEY_BYTES];
    uint8_t privatekey[PRIVATEKEY_BYTES];
    char last_idstring[MAX_ID_SPECIFIC_STRING];
    unsigned char privatekeybase64[PRIVATEKEY_BYTES * 2];
    MasterPublicKey _mk, *masterkey;
    DID last_did;

    if (publickey)
        assert(size >= PUBLICKEY_BYTES);

    assert(store);
    assert(storepass);
    assert(*storepass);
    assert(seed);

    masterkey = HDkey_GetMasterPublicKey(seed, 0, &_mk);
    if (!masterkey)
        return -1;

    index = get_last_index(store);
    last_index = index;

    while (last_index - index <= 1) {
        if (!HDkey_GetSubPublicKey(masterkey, 0, last_index, last_publickey))
            return -1;

        if (!HDkey_GetIdString(last_publickey, last_idstring, sizeof(last_idstring)))
            return -1;

        strcpy((char*)last_did.idstring, last_idstring);
        if (did && publickey && last_index == index) {
            strcpy(did->idstring, last_idstring);
            memcpy(publickey, last_publickey, size);
        }

        document = DIDStore_ResolveDID(store, &last_did, true);
        if (!document)
            last_index++;
        else {
            rc = DIDStore_StoreDID(store, document, NULL);
            if (rc < 0)
                return -1;

            DIDURL id;
            DID_Copy(&id.did, &last_did);
            strcpy(id.fragment, "primary");
            if (!HDkey_GetSubPrivateKey(seed, 0, 0, last_index, privatekey) ||
                encrypt_to_base64((char *)privatekeybase64, storepass, privatekey, sizeof(privatekey)) == -1 ||
                DIDStore_StorePrivateKey(store, &last_did, &id, (const char *)privatekeybase64) == -1) {
                DIDStore_DeleteDID(store, &last_did);
                //TODO: check need destroy document
                return -1;
            }

            index = ++last_index;
        }
    }

    return index;
}

bool DIDStore_HasPrivateIdentity(DIDStore *store)
{
    const char *encrpted_seed;
    int rc;

    if (!store)
        return false;

    rc = load_files(get_file_path(store, DIDStore_Rootkey, "private", NULL, 0),
            &encrpted_seed);
    if (rc)
        return false;

    return strlen(encrpted_seed) > 0;
}

int DIDStore_InitPrivateIdentity(DIDStore *store, const char *mnemonic,
        const char *passphrase, const char *storepass, const int language, bool force)
{
    int index;
    uint8_t seed[SEED_BYTES];
    const char *encrpted_seed;

    if (!store || !mnemonic || !storepass || !*storepass)
        return -1;

    if (!passphrase)
        passphrase = "";

    //check if DIDStore has existed private identity
    if (!load_files(get_file_path(store, DIDStore_Rootkey, "private", NULL, 0),
            &encrpted_seed) && strlen(encrpted_seed) > 0 && !force)
        return -1;

    if (!HDkey_GetSeedFromMnemonic(mnemonic, passphrase, language, seed) ||
        store_seed(store, seed, sizeof(seed), storepass) == -1)
        return -1;

    memset(seed, 0, sizeof(seed));

    index = refresh_did_fromchain(store, storepass, seed, NULL, NULL, 0);
    if (index < 0)
        return -1;

    return store_index(store, index);
}

DIDDocument *DIDStore_NewDID(DIDStore *store, const char *storepass, const char *alias)
{
    int index;
    unsigned char privatekeybase64[MAX_PRIVATEKEY_BASE64];
    uint8_t publickey[PUBLICKEY_BYTES];
    uint8_t privatekey[PRIVATEKEY_BYTES];
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    uint8_t seed[SEED_BYTES * 2];
    ssize_t len;
    DID did;
    DIDDocument *document;

    if (!store || !storepass || !*storepass)
        return NULL;

    len = load_seed(store, seed, sizeof(seed), storepass);
    if (len < 0)
        return NULL;

    index = refresh_did_fromchain(store, storepass, seed, &did, publickey, sizeof(publickey));
    if (index < 0) {
        memset(seed, 0, sizeof(seed));
        return NULL;
    }

    DIDURL id;
    DID_Copy(&id.did, &did);
    strcpy(id.fragment, "primary");
    if (!HDkey_GetSubPrivateKey(seed, 0, 0, index, privatekey) ||
        encrypt_to_base64((char *)privatekeybase64, storepass, privatekey, sizeof(privatekey)) == -1 ||
        DIDStore_StorePrivateKey(store, &did, &id, (const char *)privatekeybase64) == -1) {
        memset(seed, 0, sizeof(seed));
        return NULL;
    }

    base58_encode(publickeybase58, publickey, sizeof(publickey));
    document = create_document(&did, publickeybase58, storepass, alias);
    if (!document) {
        memset(seed, 0, sizeof(seed));
        DIDStore_DeleteDID(store, &did);
        return NULL;
    }

    if (DIDStore_StoreDID(store, document, alias) == -1) {
        memset(seed, 0, sizeof(seed));
        DIDStore_DeleteDID(store, &did);
        DIDDocument_Destroy(document);
        return NULL;
    }

    memset(seed, 0, sizeof(seed));
    memset(privatekey, 0, sizeof(privatekey));

    return document;
}

int DIDStore_Signv(DIDStore *store, DID *did, DIDURL *key, const char *storepass,
        char *sig, int count, va_list inputs)
{
    const char *privatekey;
    unsigned char binkey[PRIVATEKEY_BYTES];
    int rc;

    if (!store || !did || !key || !storepass || !*storepass || !sig || count <= 0)
        return -1;

    if (load_files(get_file_path(store, DIDStore_PrivateKey, did->idstring, key->fragment, 0),
               &privatekey) == -1)
        return -1;

    rc = decrypt_from_base64(binkey, storepass, privatekey);
    free((char*)privatekey);
    if (rc == -1)
        return -1;

    if (ecdsa_sign_base64v(sig, binkey, count, inputs) <= 0)
        return -1;

    memset(binkey, 0, sizeof(binkey));
    return 0;
}

int DIDStore_Sign(DIDStore *store, DID *did, DIDURL *key, const char *storepass,
        char *sig, int count, ...)
{
    int rc;
    va_list inputs;

    if (!store || !did || !key || !storepass || !*storepass || !sig || count <= 0)
        return -1;

    va_start(inputs, count);
    rc = DIDStore_Signv(store, did, key, storepass, sig, count, inputs);
    va_end(inputs);

    return rc;
}

DIDDocument *DIDStore_ResolveDID(DIDStore *store, DID *did, bool force)
{
    DIDDocument *document;
    const char *json;

    if (!store || !did)
        return NULL;

    document = DIDBackend_Resolve(&store->backend, did);
    if (document && DIDStore_StoreDID(store, document, "") == -1) {
        DIDDocument_Destroy(document);
        return NULL;
    }

    if (!document && !force)
        document = DIDStore_LoadDID(store, did);

    return document;
}

const char *DIDStore_PublishDID(DIDStore *store, DIDDocument *document,
        DIDURL *signKey, const char *storepass)
{
    char alias[MAX_ALIAS];

    if (!store || !document || !storepass || !*storepass)
        return NULL;

    if (DIDDocument_GetAlias(document, alias, sizeof(alias)) == -1)
        return NULL;

    if (DIDStore_StoreDID(store, document, alias) == -1)
        return NULL;

    if (!signKey)
        signKey = DIDDocument_GetDefaultPublicKey(document);

    return DIDBackend_Create(&store->backend, document, signKey, storepass);
}

const char *DIDStore_UpdateDID(DIDStore *store, DIDDocument *document,
        DIDURL *signKey, const char *storepass)
{
    char alias[MAX_ALIAS];

    if (!store || !document || !signKey || !storepass || !*storepass)
        return NULL;

    if (DIDDocument_GetAlias(document, alias, sizeof(alias)) == -1)
        return NULL;

    if (DIDStore_StoreDID(store, document, alias) == -1)
        return NULL;

    if (!signKey)
        signKey = DIDDocument_GetDefaultPublicKey(document);

    return DIDBackend_Update(&store->backend, document, signKey, storepass);
}

const char *DIDStore_DeactivateDID(DIDStore *store, DID *did, DIDURL *signKey,
        const char *storepass)
{
    if (!store || !did || !signKey || !storepass || !*storepass)
        return NULL;

    if (!signKey) {
        DIDDocument *doc = DIDStore_ResolveDID(store, did, false);
        if (!doc)
            return NULL;

        signKey = DIDDocument_GetDefaultPublicKey(doc);
        DIDDocument_Destroy(doc);
        if (!signKey)
            return NULL;
    }

    return DIDBackend_Deactivate(&store->backend, did, signKey, storepass);
}