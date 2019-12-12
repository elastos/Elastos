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

DIDStore *storeInstance = NULL;

static char *StoreTag = "/.DIDStore";
static char MAGIC[] = { 0x00, 0x0D, 0x01, 0x0D };
static char VERSION[] = { 0x00, 0x00, 0x00, 0x01 };
static const char *ProofType = "ECDSAsecp256r1";

typedef enum DIDStore_Type {
    DIDStore_Root,
    DIDStore_Rootkey,
    DIDStore_RootIndex,
    DIDStore_DID,
    DIDStore_DIDMeta,
    DIDStore_Doc,
    DIDStore_Credential,
    DIDStore_CredentialMeta,
    DIDStore_PrivateKey
} DIDStore_Type;

static const char* methods[] = { "", "key", "index", "", "", "document",
            "credentials", "credentials", "privatekeys"};

typedef struct DID_Hint {
    DIDStore *store;
    DIDStore_GetDIDHintCallback *cb;
} DID_Hint;

typedef struct Cred_Hint {
    DIDStore *store;
    DIDStore_GetCredHintCallback *cb;
    DID did;
    const char *type;
} Cred_Hint;

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
        const char *didstring, int create)
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

    if (type == DIDStore_DID || type == DIDStore_DIDMeta)
        return file_root;

    len = snprintf(file_suffix, sizeof(file_suffix), "/%s", didstring);
    if (len < 0 || len > sizeof(file_suffix))
        return NULL;

    strcat(file_root, file_suffix);

    if (create && test_path(file_root) < 0 && mkdir(file_root, S_IRWXU) == -1)
        return NULL;

    if (type == DIDStore_Doc || type == DIDStore_Rootkey || type == DIDStore_RootIndex)
       return file_root;

    len = snprintf(file_suffix, sizeof(file_suffix), "/%s", methods[type]);
    if (len < 0 || len > sizeof(file_suffix))
        return NULL;

    strcat(file_root, file_suffix);

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

    root = get_file_root(store, type, didstring, create);
    if (!root)
        return NULL;

    if (type == DIDStore_Root)
        return root;

	if (type == DIDStore_DID)
		return get_file_root(store, DIDStore_Doc, didstring, create);

	if (type == DIDStore_Doc || type == DIDStore_Rootkey || type == DIDStore_RootIndex) {
		len = snprintf(file_path, sizeof(file_path), "%s/%s", root, methods[type]);
        if (len < 0 || len > sizeof(file_path))
            return NULL;

		return file_path;
	}

    if (type == DIDStore_DIDMeta) {
        len = snprintf(file_path, sizeof(file_path), "%s/.%s.meta", root, didstring);
        if (len < 0 || len > sizeof(file_path))
            return NULL;

        return file_path;
    }

    if (!fragment)
        return NULL;

    if (type == DIDStore_CredentialMeta) {
        len = snprintf(file_path, sizeof(file_path), "%s/.%s.meta", root, fragment);
        if (len < 0 || len > sizeof(file_path))
            return NULL;

        return file_path;
    }

    len = snprintf(file_path, sizeof(file_path), "%s/%s", root, fragment);
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

    assert(path);
    assert(path);

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

static int store_meta_file(DIDStore *store, DIDStore_Type type, const char *didstring,
        const char *fragment, const char *hint)
{
    const char *path;

    if (type == DIDStore_Root || type == DIDStore_Doc || type == DIDStore_PrivateKey || !didstring)
        return -1;

    path = get_file_path(store, type, didstring, fragment, 1);
    if (!path)
        return -1;

    if (!hint || !strlen(hint)) {
        delete_file(path);
        return 0;
    }

    if (test_path(path) == S_IFDIR)
        delete_file(path);

    if (store_file(path, hint) == -1)
        return -1;

    return 0;
}

static const char *load_meta_file(DIDStore *store, DIDStore_Type type,
        const char *didstring, const char *fragment)
{
    const char *hint;
    const char *path;
    int rc;

    if (type == DIDStore_Root || type == DIDStore_Doc ||
            type == DIDStore_PrivateKey || !didstring)
        return NULL;

    path = get_file_path(store, type, didstring, fragment, 0);
    if (!path)
        return NULL;

    rc = test_path(path);
    if (rc < 0)
        return NULL;
    else if (rc == S_IFDIR) {
        delete_file(path);
        return NULL;
    }

    if (load_files(path, &hint) == -1)
        return NULL;

    return hint;
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
    DID_Hint *dh = (DID_Hint*)context;
    DIDEntry *entry;
    const char* hint;

    if (strcmp(path, ".") == 0 || strcmp(path, "..") == 0)
        return 0;

    entry = (DIDEntry*)calloc(1, sizeof(DIDEntry));
    if (!entry)
        return -1;

    strcpy((char*)entry->did.idstring, path);
    hint = load_meta_file(dh->store, DIDStore_DIDMeta, path, NULL);
    if (hint) {
        strcpy((char*)entry->hint, hint);
        free((char*)hint);
    }
    else
        strcpy((char*)entry->hint, "");

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
    const char* string, *hint;
    Credential *credential;
    CredentialEntry *entry;

    Cred_Hint *ch = (Cred_Hint*)context;

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
            hint = load_meta_file(ch->store, DIDStore_Credential, ch->did.idstring, path);
            if (hint) {
                strcpy((char*)entry->hint, hint);
                free((char*)hint);
            }
            else
                strcpy((char*)entry->hint, "");

            return ch->cb(entry, NULL);
        }
    }
    return -1;
}

static int list_credential_helper(const char *path, void *context)
{
    Cred_Hint *ch = (Cred_Hint*)context;
    CredentialEntry *cred_entry;
    DIDURL *id;
    const char *hint;

    assert(path);

    if (strstr(path, ".") == 0 || strstr(path, "..") == 0)
        return 0;

    cred_entry = (CredentialEntry*)calloc(1, sizeof(CredentialEntry));
    if (!cred_entry)
        return -1;

    strcpy((char*)cred_entry->id.did.idstring, ch->did.idstring);
    strcpy((char*)cred_entry->id.fragment, path);
    hint = load_meta_file(ch->store, DIDStore_CredentialMeta, ch->did.idstring, path);
    if (hint) {
        strcpy((char*)cred_entry->hint, hint);
        free((char*)hint);
    }
    else
        strcpy((char*)cred_entry->hint, "");

    return ch->cb(cred_entry, NULL);
}

static DIDDocument *create_document(DID *did, const char *key, const char *storepass)
{
    PublicKey *publickey;
    DIDURL id;
    DID controller;
    const char *data;
    int rc;
    char signature[SIGNATURE_BYTES * 2];

    assert(did);
    assert(key);

    DIDDocument *document = (DIDDocument*)calloc(1, sizeof(DIDDocument));
    if (!document)
        return NULL;

    if (DID_Copy(&document->did, did) == -1) {
        DIDDocument_Destroy(document);
        return NULL;
    }

    strcpy((char*)controller.idstring, did->idstring);
    strcpy((char*)id.did.idstring, did->idstring);
    strcpy((char*)id.fragment, "primary");

    if (DIDDocument_AddPublicKey(document, &id, &controller, key) == -1) {
        DIDDocument_Destroy(document);
        return NULL;
    }

    if (DIDDocument_AddAuthenticationKey(document, &id, key) == -1) {
        DIDDocument_Destroy(document);
        return NULL;
    }

    DIDDocument_SetExpires(document, 0);

    data = DIDDocument_ToJson(document, 0, 1);
    if (!data) {
        DIDDocument_Destroy(document);
        return NULL;
    }

    rc = DIDDocument_Sign(document, &id, storepass, signature, 1,
            (unsigned char*)data, strlen(data));
    free((char*)data);
    if (rc) {
        DIDDocument_Destroy(document);
        return NULL;
    }

    strcpy(document->proof.type, ProofType);
    time(&document->proof.created);
    DIDURL_Copy(&document->proof.creater, &id);
    strcpy(document->proof.signatureValue, signature);

    return document;
}

/////////////////////////////////////////////////////////////////////////
DIDStore* DIDStore_Initialize(const char *root, DIDAdapter *adapter)
{
    if (!root || !adapter)
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

int DIDStore_SetDIDHint(DIDStore *store, DID *did, const char *hint)
{
    return store_meta_file(store, DIDStore_DIDMeta, did->idstring, NULL, hint);
}

const char *DIDStore_GetDIDHint(DIDStore *store, DID *did)
{
    return load_meta_file(store, DIDStore_DIDMeta, did->idstring, NULL);
}

int DIDStore_StoreDID(DIDStore *store, DIDDocument *document, const char *hint)
{
    const char *path, *string;

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
        if (store_meta_file(store, DIDStore_DIDMeta, document->did.idstring, NULL, hint) == 0)
            return 0;
		else {
			delete_file(path);
			const char *did_path = get_file_root(store, DIDStore_DID, document->did.idstring, 0);
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

int DIDStore_ListDID(DIDStore *store, DIDStore_GetDIDHintCallback *callback,
        void *context)
{
    ssize_t size = 0;
    char *dir_path;
    DID_Hint dh;
    int rc;

    if (!store || !callback)
        return -1;

    dir_path = store->root;
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

int DIDStore_StoreCredential(DIDStore *store, Credential *credential, const char *hint)
{
    const char *path, *data;

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
        if(store_meta_file(store, DIDStore_CredentialMeta,
                credential->id.did.idstring, credential->id.fragment, hint) == 0)
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

int DIDStore_SetCredentialHint(DIDStore *store, DID *did, DIDURL *id, const char *hint)
{
    char _id[MAX_DID];

    return store_meta_file(store, DIDStore_CredentialMeta,
            DID_ToString(did, _id, sizeof(_id)), id->fragment, hint);
}

const char *DIDStore_GetCredentialHint(DIDStore *store, DID *did, DIDURL *id)
{
    char _id[MAX_DID];

    return load_meta_file(store, DIDStore_CredentialMeta,
            DID_ToString(did, _id, sizeof(_id)), id->fragment);
}

bool DIDStore_ContainsCredentials(DIDStore *store, DID *did)
{
    const char *path;
    int rc;

    if (!store || !did)
        return false;

    path = get_file_root(store, DIDStore_Credential, did->idstring, 0);
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

    if (!store || !did || !id || !DIDStore_ContainsCredentials(store, did))
        return;

    path = get_file_path(store, DIDStore_Credential, did->idstring, id->fragment, 0);
    if (test_path(path) > 0)
        delete_file(path);

    meta_path = get_file_path(store, DIDStore_CredentialMeta, did->idstring, id->fragment, 0);
    if (test_path(meta_path) > 0)
        delete_file(meta_path);

    root = get_file_root(store, DIDStore_Credential, did->idstring, 0);
    if (root && !is_empty(root))
        delete_file(root);

    return;
}

int DIDStore_ListCredentials(DIDStore *store, DID *did,
        DIDStore_GetCredHintCallback *callback, void *context)
{
    ssize_t size = 0;
    const char *dir_path;
    Cred_Hint ch;
    int rc;

    if (!store || !did || !callback)
        return -1;

    dir_path = get_file_root(store, DIDStore_Credential, did->idstring, 0);
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
        const char *type, DIDStore_GetCredHintCallback *callback, void *context)
{
    const char *path = NULL, *meta_path = NULL, *hint;
    CredentialEntry *entry;
    Cred_Hint ch;
    int rc;

    if (!store || !did || (!id && !type) || !callback)
        return -1;

    if (id) {
        path = get_file_path(store, DIDStore_Credential, did->idstring, id->fragment, 0);
        if (test_path(path) > 0) {
            if ((type && has_type(did, path, type) == true) || !type) {
                meta_path = get_file_path(store, DIDStore_CredentialMeta, did->idstring, id->fragment, 0);
                if (meta_path && load_files(meta_path, &hint) == 0) {
                    strcpy((char*)entry->hint, hint);
                    free((char*)hint);
                }
                else
                    strcpy((char*)entry->hint, "");

                strcpy((char*)entry->id.did.idstring, id->did.idstring);
                strcpy((char*)entry->id.fragment, id->fragment);
                callback(entry, NULL);
                return 0;
            }
            return -1;
        }
        return -1;
    }

    path = get_file_root(store, DIDStore_Credential, did->idstring, 0);

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

    dir_path = get_file_root(store, DIDStore_PrivateKey, did->idstring, 0);

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

int DIDStore_StorePrivateKey(DIDStore *store, DID *did, const char *fragment,
        const char *privatekey)
{
    size_t len;
    const char *path;

    if (!store || !did || !fragment || !privatekey)
        return -1;

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
    assert(did);

    masterkey = HDkey_GetMasterPublicKey(seed, 0, &_mk);
    if (!masterkey)
        return -1;

    index = get_last_index(store);
    last_index = index;

    while (last_index - index <= 20) {
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

            if (!HDkey_GetSubPrivateKey(seed, 0, 0, last_index, privatekey) ||
                encrypt_to_base64((char *)privatekeybase64, storepass, privatekey, sizeof(privatekey)) == -1 ||
                DIDStore_StorePrivateKey(store, &last_did, "primary", (const char *)privatekeybase64) == -1) {
                DIDStore_DeleteDID(store, &last_did);
                //TODO: check need destroy document
                return -1;
            }

            index = ++last_index;
        }
    }

    return index;
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

DIDDocument *DIDStore_NewDID(DIDStore *store, const char *storepass, const char *hint)
{
    int index;
    unsigned char privatekeybase64[PRIVATEKEY_BYTES * 2];
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

    base58_encode(publickeybase58, publickey, sizeof(publickey));

    document = create_document(&did, publickeybase58, storepass);
    if (!document) {
        memset(seed, 0, sizeof(seed));
        return NULL;
    }

    if (DIDStore_StoreDID(store, document, hint) == -1) {
        memset(seed, 0, sizeof(seed));
        DIDDocument_Destroy(document);
        return NULL;
    }

    if (!HDkey_GetSubPrivateKey(seed, 0, 0, index, privatekey) ||
        encrypt_to_base64((char *)privatekeybase64, storepass, privatekey, sizeof(privatekey)) == -1 ||
        DIDStore_StorePrivateKey(store, &did, "primary", (const char *)privatekeybase64) == -1) {
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
    char id[MAX_DID];

    if (!store || !did)
        return NULL;

    document = DIDBackend_Resolve(&store->backend, did);
    if (document && DIDStore_StoreDID(store, document, NULL) == -1) {
        DIDDocument_Destroy(document);
        return NULL;
    }

    if (!document && !force)
        document = DIDStore_LoadDID(store, did);

    return document;
}

int DIDStore_PublishDID(DIDStore *store, DIDDocument *document,
        DIDURL *signKey, const char *storepass)
{
    if (!store || !document || !storepass || !*storepass)
        return -1;

    if (DIDStore_StoreDID(store, document, "") == -1)
        return -1;

    if (!signKey)
        signKey = DIDDocument_GetDefaultPublicKey(document);

    return DIDBackend_Create(&store->backend, document, signKey, storepass);
}

int DIDStore_UpdateDID(DIDStore *store, DIDDocument *document,
        DIDURL *signKey, const char *storepass)
{
    if (!store || !document || !signKey || !storepass || !*storepass)
        return -1;

    if (!signKey)
        signKey = DIDDocument_GetDefaultPublicKey(document);

    if (DIDStore_StoreDID(store, document, "") == -1)
        return -1;

    return DIDBackend_Update(&store->backend, document, signKey, storepass);
}

int DIDStore_DeactivateDID(DIDStore *store, DID *did, DIDURL *signKey,
        const char *storepass)
{
    if (!store || !did || !signKey || !storepass || !*storepass)
        return -1;

    if (!signKey) {
        DIDDocument *doc = DIDStore_ResolveDID(store, did, false);
        if (!doc)
            return -1;

        signKey = DIDDocument_GetDefaultPublicKey(doc);
        DIDDocument_Destroy(doc);
        if (!signKey)
            return -1;
    }

    return DIDBackend_Deactivate(&store->backend, did, signKey, storepass);
}