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
#include <fcntl.h>
#include <assert.h>
#include <sys/stat.h>
#include <openssl/opensslv.h>
#include <cjson/cJSON.h>

#include "ela_did.h"
#include "diderror.h"
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

#define MAX_PUBKEY_BASE58            128

static const char *META_FILE = ".meta";
static char MAGIC[] = { 0x00, 0x0D, 0x01, 0x0D };
static char VERSION[] = { 0x00, 0x00, 0x00, 0x02 };

static const char *PATH_SEP = "/";
static const char *PRIVATE_DIR = "private";
static const char *HDKEY_FILE = "key";
static const char *HDPUBKEY_FILE = "key.pub";
static const char *INDEX_FILE = "index";
static const char *MNEMONIC_FILE = "mnemonic";

static const char *DID_DIR = "ids";
static const char *DOCUMENT_FILE = "document";
static const char *CREDENTIALS_DIR = "credentials";
static const char *CREDENTIAL_FILE = "credential";
static const char *PRIVATEKEYS_DIR = "privatekeys";

static const char *PRIVATE_JOURNAL = "private.journal";
static const char *DID_JOURNAL = "ids.journal";

static const char *KEY_PATH = "/private/key";
static const char *MNEMONIC_PATH = "/private/mnemonic";
static const char *POST_PASSWORD = "postChangePassword";

extern const char *ProofType;

typedef struct DID_List_Helper {
    DIDStore *store;
    DIDStore_DIDsCallback *cb;
    void *context;
    int filter;
} DID_List_Helper;

typedef struct Cred_List_Helper {
    DIDStore *store;
    DIDStore_CredentialsCallback *cb;
    void *context;
    DID did;
    const char *type;
} Cred_List_Helper;

typedef struct Dir_Copy_Helper {
    const char *srcpath;
    const char *dstpath;
    const char *oldpassword;
    const char *newpassword;
} Dir_Copy_Helper;

static int store_didmeta(DIDStore *store, DIDMeta *meta, DID *did)
{
    char path[PATH_MAX];
    const char *data;
    int rc;

    assert(store);
    assert(meta);
    assert(did);

    if (DIDMeta_IsEmpty(meta)) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "No did meta.");
        return 0;
    }

    if (get_file(path, 1, 4, store->root, DID_DIR, did->idstring, META_FILE) == -1) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Create file for didmeta file failed.");
        return -1;
    }

    if (test_path(path) == S_IFDIR) {
        DIDError_Set(DIDERR_IO_ERROR, "Did meta should be a file.");
        delete_file(path);
        return -1;
    }

    data = DIDMeta_ToJson(meta);
    if (!data) {
        delete_file(path);
        return -1;
    }

    rc = store_file(path, data);
    free((char*)data);
    if (rc)
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Store did meta failed.");

    return rc;
}

static int load_didmeta(DIDStore *store, DIDMeta *meta, const char *did)
{
    const char *data;
    char path[PATH_MAX];
    int rc;

    assert(store);
    assert(meta);
    assert(did);

    memset(meta, 0, sizeof(DIDMeta));
    if (get_file(path, 0, 4, store->root, DID_DIR, did, META_FILE) == -1) {
        DIDError_Set(DIDERR_NOT_EXISTS, "Did meta don't exist.");
        return 0;
    }

    rc = test_path(path);
    if (rc < 0) {
        DIDError_Set(DIDERR_IO_ERROR, "File error.");
        return 0;
    }

    if (rc == S_IFDIR) {
        DIDError_Set(DIDERR_IO_ERROR, "Did meta should be a file.");
        delete_file(path);
        return -1;
    }

    data = load_file(path);
    if (!data) {
        DIDError_Set(DIDERR_IO_ERROR, "Load did meta failed.");
        return -1;
    }

    rc = DIDMeta_FromJson(meta, data);
    free((char*)data);
    if (rc)
        return rc;

    DIDMeta_SetStore(meta, store);
    return rc;
}

int DIDStore_LoadDIDMeta(DIDStore *store, DIDMeta *meta, DID *did)
{
    bool iscontain;

    assert(store);
    assert(meta);
    assert(did);

    iscontain = DIDStore_ContainsDID(store, did);
    if (!iscontain) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "No did: %s", did->idstring);
        return -1;
    }

    return load_didmeta(store, meta, did->idstring);
}

int DIDStore_StoreDIDMeta(DIDStore *store, DIDMeta *meta, DID *did)
{
    bool iscontain;

    assert(store);
    assert(meta);
    assert(did);

    iscontain = DIDStore_ContainsDID(store, did);
    if (!iscontain) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "No DID: %s", did->idstring);
        return -1;
    }

    return store_didmeta(store, meta, did);
}

static int store_credmeta(DIDStore *store, CredentialMeta *meta, DIDURL *id)
{
    char path[PATH_MAX];
    const char *data;
    int rc;

    assert(store);
    assert(meta);
    assert(id);

    if (CredentialMeta_IsEmpty(meta))
        return 0;

    data = CredentialMeta_ToJson(meta);
    if (!data)
        return -1;

    if (get_file(path, 1, 6, store->root, DID_DIR, id->did.idstring,
            CREDENTIALS_DIR, id->fragment, META_FILE) == -1) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Create file for credential meta failed.");
        free((char*)data);
        return -1;
    }

    if (test_path(path) == S_IFDIR) {
        DIDError_Set(DIDERR_IO_ERROR, "Credential meta should be a file.");
        free((char*)data);
        goto errorExit;
    }

    rc = store_file(path, data);
    free((char*)data);
    if (!rc)
        return 0;

errorExit:
    delete_file(path);

    if (get_dir(path, 0, 5, store->root, DID_DIR, id->did.idstring,
            CREDENTIALS_DIR, id->fragment) == 0) {
        if (is_empty(path))
            delete_file(path);
    }

    if (get_dir(path, 0, 4, store->root, DID_DIR, id->did.idstring, CREDENTIALS_DIR) == 0) {
        if (is_empty(path))
            delete_file(path);
    }

    DIDError_Set(DIDERR_DIDSTORE_ERROR, "Store credential meta failed.");
    return -1;
}

static int load_credmeta(DIDStore *store, CredentialMeta *meta, const char *did,
        const char *fragment)
{
    const char *data;
    char path[PATH_MAX];
    int rc;

    assert(store);
    assert(meta);
    assert(did);
    assert(fragment);

    memset(meta, 0, sizeof(CredentialMeta));
    if (get_file(path, 0, 6, store->root, DID_DIR, did, CREDENTIALS_DIR,
            fragment, META_FILE) == -1)
        return 0;

    rc = test_path(path);
    if (rc < 0) {
        DIDError_Set(DIDERR_IO_ERROR, "File error.");
        return 0;
    }

    if (rc == S_IFDIR) {
        DIDError_Set(DIDERR_IO_ERROR, "Credential meta should be file.");
        delete_file(path);
        return -1;
    }

    data = load_file(path);
    if (!data) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Store credential meta error.");
        return -1;
    }

    rc = CredentialMeta_FromJson(meta, data);
    free((char*)data);
    if (rc)
        return rc;

    CredentialMeta_SetStore(meta, store);
    return rc;
}

int DIDStore_StoreCredMeta(DIDStore *store, CredentialMeta *meta, DIDURL *id)
{
    bool iscontain;

    assert(store);
    assert(meta);
    assert(id);

    iscontain = DIDStore_ContainsCredential(store, DIDURL_GetDid(id), id);
    if (!iscontain) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "No credential: %s#%s", id->did.idstring, id->fragment);
        return -1;
    }

    return store_credmeta(store, meta, id);
}

int DIDStore_LoadCredMeta(DIDStore *store, CredentialMeta *meta, DIDURL *id)
{
    bool iscontain;

    assert(store);
    assert(meta);
    assert(id);

    iscontain = DIDStore_ContainsCredential(store, DIDURL_GetDid(id), id);
    if (!iscontain) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "No credential: %s#%s", id->did.idstring, id->fragment);
        return -1;
    }

    return load_credmeta(store, meta, id->did.idstring, id->fragment);
}

static int create_store(DIDStore *store)
{
    int fd;
    size_t size;
    char path[PATH_MAX];

    assert(store);

    if (get_file(path, 1, 2, store->root, META_FILE) == -1)
        return -1;

    fd = open(path, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
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

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-overflow="
#endif

static int postChangePassword(DIDStore *store)
{
    char post_file[PATH_MAX];
    char private_dir[PATH_MAX], private_journal[PATH_MAX], private_deprecated[PATH_MAX];
    char did_dir[PATH_MAX], did_journal[PATH_MAX], did_deprecated[PATH_MAX];

    assert(store);

    sprintf(post_file, "%s/%s", store->root, POST_PASSWORD);
    if(test_path(post_file) == S_IFREG) {
        if (get_dir(private_journal, 0, 2, store->root, PRIVATE_JOURNAL) == 0) {
            if (get_dir(private_dir, 0, 2, store->root, PRIVATE_DIR) == 0) {
                sprintf(private_deprecated, "%s/%s", store->root, "private.deprecated");
                delete_file(private_deprecated);
                rename(private_dir, private_deprecated);
            }

            rename(private_journal, private_dir);
        }

        if (get_dir(did_journal, 0, 2, store->root, DID_JOURNAL) == 0) {
            if (get_dir(did_dir, 0, 2, store->root, DID_DIR) == 0) {
                sprintf(did_deprecated, "%s/%s", store->root, "ids.deprecated");
                delete_file(did_deprecated);
                rename(did_dir, did_deprecated);
            }

            rename(did_journal, did_dir);
        }

        delete_file(private_deprecated);
        delete_file(did_deprecated);
        delete_file(post_file);
    } else {
        if (get_dir(private_journal, 0, 2, store->root, PRIVATE_JOURNAL) == 0)
            delete_file(private_journal);
        if (get_dir(did_journal, 0, 2, store->root, DID_JOURNAL) == 0)
            delete_file(did_journal);
    }
    return 0;
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

static int check_store(DIDStore *store)
{
    int fd;
    char symbol[1];
    int i, flag = 0;
    size_t size;
    char path[PATH_MAX];

    assert(store);

    if (test_path(store->root) != S_IFDIR)
        return -1;

    if (get_file(path, 0, 2, store->root, META_FILE) == -1)
        return -1;

    fd = open(path, O_RDONLY);
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

    return postChangePassword(store);
}

static int store_extendedprvkey(DIDStore *store, uint8_t *extendedkey,
        size_t size, const char *storepass)
{
    unsigned char base64[512];
    char path[PATH_MAX];

    assert(store);
    assert(extendedkey && size > 0);
    assert(storepass);
    assert(*storepass);

    if (encrypt_to_base64((char *)base64, storepass, extendedkey, size) == -1) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Encrypt extended private key failed.");
        return -1;
    }

    if (get_file(path, 1, 3, store->root, PRIVATE_DIR, HDKEY_FILE) == -1) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Create file for extended private key failed.");
        return -1;
    }

    if (store_file(path, (const char *)base64) == -1) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Store extended private key failed.");
        return -1;
    }

    return 0;
}

static ssize_t load_extendedprvkey(DIDStore *store, uint8_t *extendedkey, size_t size,
        const char *storepass)
{
    const char *string;
    char path[PATH_MAX];
    ssize_t len;

    assert(store);
    assert(extendedkey && size >= EXTENDEDKEY_BYTES);
    assert(storepass);
    assert(*storepass);

    if (get_file(path, 0, 3, store->root, PRIVATE_DIR, HDKEY_FILE) == -1) {
        DIDError_Set(DIDERR_NOT_EXISTS, "extended private key don't already exist.");
        return -1;
    }

    string = load_file(path);
    if (!string) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Load extended private key failed.");
        return -1;
    }

    len = decrypt_from_base64(extendedkey, storepass, string);
    free((char*)string);
    if (len < 0)
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Decrypt extended private key failed.");

    return len;
}

static int store_extendedpubkey(DIDStore *store, uint8_t *extendedkey, size_t size)
{
    char publickeybase58[MAX_PUBKEY_BASE58];
    char path[PATH_MAX];

    assert(store);
    assert(extendedkey && size > 0);

    if (base58_encode(publickeybase58, sizeof(publickeybase58), extendedkey, size) == -1) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Decode extended public key failed.");
        return -1;
    }

    if (get_file(path, 1, 3, store->root, PRIVATE_DIR, HDPUBKEY_FILE) == -1) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Create file for extended public key failed.");
        return -1;
    }

    if (store_file(path, (const char *)publickeybase58) == -1) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Store extended public key failed.");
        return -1;
    }

    return 0;
}

static ssize_t load_extendedpubkey(DIDStore *store, uint8_t *extendedkey, size_t size)
{
    const char *string;
    char path[PATH_MAX];
    ssize_t len;

    assert(store);
    assert(extendedkey && size >= EXTENDEDKEY_BYTES);

    if (get_file(path, 0, 3, store->root, PRIVATE_DIR, HDPUBKEY_FILE) == -1) {
        DIDError_Set(DIDERR_NOT_EXISTS, "Extended public key don't exist.");
        return -1;
    }

    string = load_file(path);
    if (!string) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Load extended public key failed.");
        return -1;
    }

    len = base58_decode(extendedkey, size, string);
    free((char*)string);
    if (len < 0)
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Decode extended public key failed.");

    return len;
}

static int store_mnemonic(DIDStore *store, const char *storepass, const char *mnemonic)
{
    char base64[512];
    char path[PATH_MAX];

    assert(store);
    assert(mnemonic);
    assert(*mnemonic);

    if (encrypt_to_base64(base64, storepass, (const uint8_t*)mnemonic, strlen(mnemonic)) == -1) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Encrypt mnemonic failed.");
        return -1;
    }

    if (get_file(path, 1, 3, store->root, PRIVATE_DIR, MNEMONIC_FILE) == -1) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Create file for mnemonic failed.");
        return -1;
    }

    if (store_file(path, base64) == -1) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Store mnemonic failed.");
        return -1;
    }

    return 0;
}

static ssize_t load_mnemonic(DIDStore *store, const char *storepass,
        char *mnemonic, size_t size)
{
    const char *encrpted_mnemonic;
    char path[PATH_MAX];
    ssize_t len;

    assert(store);
    assert(mnemonic);
    assert(size >= ELA_MAX_MNEMONIC_LEN);

    if (get_file(path, 0, 3, store->root, PRIVATE_DIR, MNEMONIC_FILE) == -1) {
        DIDError_Set(DIDERR_NOT_EXISTS, "Mnemonic file don't exist.");
        return -1;
    }

    encrpted_mnemonic = load_file(path);
    if (!encrpted_mnemonic) {
        DIDError_Set(DIDERR_IO_ERROR, "File error.");
        return -1;
    }

    len = decrypt_from_base64((uint8_t*)mnemonic, storepass, encrpted_mnemonic);
    free((char*)encrpted_mnemonic);
    if (len < 0)
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Decrypt mnemonic failed.");

    return len;
}

static int load_index(DIDStore *store)
{
    char path[PATH_MAX];
    const char *string;
    int index;

    assert(store);

    if (get_file(path, 0, 3, store->root, PRIVATE_DIR, INDEX_FILE) == -1) {
        DIDError_Set(DIDERR_NOT_EXISTS, "Index file is not exist.");
        return -1;
    }

    string = load_file(path);
    if (!string) {
        DIDError_Set(DIDERR_IO_ERROR, "Load index failed.");
        return -1;
    }

    index = atoi(string);
    free((char*)string);
    return index;
}

static int store_index(DIDStore *store, int index)
{
    char path[PATH_MAX];
    char string[32];
    int len;

    assert(store);
    assert(index >= 0);

    len = snprintf(string, sizeof(string), "%d", index);
    if (len < 0 || len > sizeof(string)) {
        DIDError_Set(DIDERR_UNKNOWN, "Unknown error.");
        return -1;
    }

    if (get_file(path, 1, 3, store->root, PRIVATE_DIR, INDEX_FILE) == -1) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Create file for index failed.");
        return -1;
    }

    if (store_file(path, string) == -1) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Store index failed.");
        return -1;
    }

    return 0;
}

static int list_did_helper(const char *path, void *context)
{
    DID_List_Helper *dh = (DID_List_Helper*)context;
    char didpath[PATH_MAX];
    DID did;
    int rc;

    if (!path)
        return dh->cb(NULL, dh->context);

    if (strcmp(path, ".") == 0 || strcmp(path, "..") == 0)
        return 0;

    if (strlen(path) >= sizeof(did.idstring)) {
        if (get_dir(didpath, 0, 3, dh->store->root, DID_DIR, path) == 0) {
            delete_file(didpath);
            return 0;
        }
    }

    strcpy(did.idstring, path);
    load_didmeta(dh->store, &did.meta, did.idstring);

    if (dh->filter == 0 || (dh->filter == 1 && DIDSotre_ContainsPrivateKeys(dh->store, &did)) ||
            (dh->filter == 2 && !DIDSotre_ContainsPrivateKeys(dh->store, &did)))
        return dh->cb(&did, dh->context);

    return 0;
}

static bool has_type(DID *did, const char *path, const char *type)
{
    const char *data;
    Credential *credential;
    int i;

    assert(did);
    assert(path);
    assert(type);

    data = load_file(path);
    if (!data)
        return false;

    credential = Credential_FromJson(data, did);
    free((char*)data);
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
    Cred_List_Helper *ch = (Cred_List_Helper*)context;
    const char* data;
    Credential *credential;
    char credpath[PATH_MAX];
    DIDURL id;

    if (!path)
        return ch->cb(NULL, ch->context);

    if (strcmp(path, ".") == 0 || strcmp(path, "..") == 0)
        return -1;

    if (get_file(credpath, 0, 6, ch->store->root, DID_DIR, ch->did.idstring,
            CREDENTIALS_DIR, path, CREDENTIAL_FILE) == -1)
        return -1;

    data = load_file(path);
    if (!data)
        return -1;

    credential = Credential_FromJson(data, &(ch->did));
    free((char*)data);
    if (!credential)
        return -1;

    for (int j = 0; j < credential->type.size; j++) {
        const char *new_type = credential->type.types[j];
        if (!new_type)
            continue;
        if (strcmp(new_type, ch->type) == 0) {
            strcpy(id.did.idstring, ch->did.idstring);
            strcpy(id.fragment, path);
            Credential_Destroy(credential);
            return ch->cb(&id, ch->context);
        }
    }
    Credential_Destroy(credential);
    return 0;
}

static int list_credential_helper(const char *path, void *context)
{
    Cred_List_Helper *ch = (Cred_List_Helper*)context;
    char credpath[PATH_MAX];
    DIDURL id;
    int rc;

    if (!path)
        return ch->cb(NULL, ch->context);

    if (strcmp(path, ".") == 0 || strcmp(path, "..") == 0)
        return 0;

    if (strlen(path) >= sizeof(id.fragment)) {
        if (get_dir(credpath, 0, 5, ch->store->root, DID_DIR, ch->did.idstring,
                CREDENTIALS_DIR, path) == 0) {
            delete_file(credpath);
            return 0;
        }
    }

    strcpy(id.did.idstring, ch->did.idstring);
    strcpy(id.fragment, path);
    load_credmeta(ch->store, &id.meta, id.did.idstring, id.fragment);
    return ch->cb(&id, ch->context);
}

static DIDDocumentBuilder* did_createbuilder(DID *did, DIDStore *store)
{
    DIDDocumentBuilder *builder;

    builder = (DIDDocumentBuilder*)calloc(1, sizeof(DIDDocumentBuilder));
    if (!builder) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Malloc buffer for document builder failed.");
        return NULL;
    }

    builder->document = (DIDDocument*)calloc(1, sizeof(DIDDocument));
    if (!builder->document) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Malloc buffer for document failed.");
        free(builder);
        return NULL;
    }

    if (!DID_Copy(&builder->document->did, did)) {
        free(builder->document);
        free(builder);
        return NULL;
    }

    DIDDocument_SetStore(builder->document, store);

    return builder;
}

static DIDDocument *create_document(DIDStore *store, DID *did, const char *key,
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

    if (Init_DIDURL(&id, did, "primary") == -1)
        return NULL;

    builder = did_createbuilder(did, store);
    if (!builder)
        return NULL;

    if (DIDDocumentBuilder_AddPublicKey(builder, &id, did, key) == -1) {
        DIDDocumentBuilder_Destroy(builder);
        return NULL;
    }

    if (DIDDocumentBuilder_AddAuthenticationKey(builder, &id, key) == -1) {
        DIDDocumentBuilder_Destroy(builder);
        return NULL;
    }

    if (DIDDocumentBuilder_SetExpires(builder, 0) == -1) {
        DIDDocumentBuilder_Destroy(builder);
        return NULL;
    }

    document = DIDDocumentBuilder_Seal(builder, storepass);
    if (!document) {
        DIDDocumentBuilder_Destroy(builder);
        return NULL;
    }

    if (DIDMeta_Init(&document->meta, alias, NULL,
            document->proof.signatureValue, false, 0) == -1) {
        DIDDocument_Destroy(document);
        return NULL;
    }

    DIDMeta_Copy(&document->did.meta, &document->meta);
    return document;
}

static int store_credential(DIDStore *store, Credential *credential)
{
    const char *data;
    char path[PATH_MAX];
    DIDURL *id;
    int rc;

    assert(store);
    assert(credential);

    id = Credential_GetId(credential);
    if (!id)
        return -1;

    data = Credential_ToJson(credential, true);
    if (!data)
        return -1;

    if (get_file(path, 1, 6, store->root, DID_DIR, id->did.idstring,
            CREDENTIALS_DIR, id->fragment, CREDENTIAL_FILE) == -1) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Create credential file failed.");
        free((char*)data);
        return -1;
    }

    rc = store_file(path, data);
    free((char*)data);
    if (!rc)
        return 0;

    delete_file(path);

    if (get_dir(path, 0, 5, store->root, DID_DIR, id->did.idstring,
            CREDENTIALS_DIR, id->fragment) == 0) {
        if (is_empty(path))
            delete_file(path);
    }

    if (get_dir(path, 0, 4, store->root, DID_DIR, id->did.idstring, CREDENTIALS_DIR) == 0) {
        if (is_empty(path))
            delete_file(path);
    }

    DIDError_Set(DIDERR_DIDSTORE_ERROR, "Store credential failed.");
    return -1;
}

/////////////////////////////////////////////////////////////////////////
DIDStore* DIDStore_Open(const char *root, DIDAdapter *adapter)
{
    char path[PATH_MAX];
    DIDStore *didstore;

    if (!root || !*root || !adapter) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }
    if (strlen(root) >= PATH_MAX) {
        DIDError_Set(DIDERR_INVALID_ARGS, "DIDStore root is too long.");
        return NULL;
    }

    didstore = (DIDStore *)calloc(1, sizeof(DIDStore));
    if (!didstore) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Malloc buffer for didstore failed.");
        return NULL;
    }

    strcpy(didstore->root, root);
    didstore->backend.adapter = adapter;

    if (get_dir(path, 0, 1, root) == 0 && !check_store(didstore))
        return didstore;

    if (get_dir(path, 1, 1, root) == 0 && !create_store(didstore))
        return didstore;

    DIDError_Set(DIDERR_DIDSTORE_ERROR, "Open DIDStore failed.");
    free(didstore);
    return NULL;
}

void DIDStore_Close(DIDStore *didstore)
{
    if (didstore)
        free(didstore);
}

int DIDStore_ExportMnemonic(DIDStore *store, const char *storepass,
        char *mnemonic, size_t size)
{
    if (!store || !storepass || !*storepass || !mnemonic || size <= 0) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    return load_mnemonic(store, storepass, mnemonic, size);
}

int DIDStore_StoreDID(DIDStore *store, DIDDocument *document, const char *alias)
{
    char path[PATH_MAX];
    const char *data, *root;
    DIDMeta meta;
    ssize_t count;
    int rc;

    if (!store || !document) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    if (load_didmeta(store, &meta, document->did.idstring) == -1)
        return -1;

    if (alias && (DIDMeta_SetAlias(&document->meta, alias) == -1 ||
            DIDMeta_Merge(&meta, &document->meta) == -1))
        return -1;

    if (!DIDMeta_IsEmpty(&meta))
        memcpy(&document->meta, &meta, sizeof(DIDMeta));

    DIDDocument_SetStore(document, store);

	data = DIDDocument_ToJson(document, true);
	if (!data)
		return -1;

    rc = get_file(path, 1, 4, store->root, DID_DIR, document->did.idstring, DOCUMENT_FILE);
    if (rc < 0) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Create file for did document failed.");
        free((char*)data);
        return -1;
    }

    rc = store_file(path, data);
    free((char*)data);
    if (rc) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Store did document failed.");
        goto errorExit;
    }

    if (store_didmeta(store, &document->meta, &document->did) == -1)
        goto errorExit;

    count = DIDDocument_GetCredentialCount(document);
    for (int i = 0; i < count; i++) {
        Credential *cred = document->credentials.credentials[i];
        store_credential(store, cred);
    }

    return 0;

errorExit:
    delete_file(path);

    //check ids directory is empty or not
    if (get_dir(path, 0, 3, store->root, DID_DIR, document->did.idstring) == 0) {
        if (is_empty(path))
            delete_file(path);
    }

    return -1;
}

DIDDocument *DIDStore_LoadDID(DIDStore *store, DID *did)
{
    DIDDocument *document;
    char path[PATH_MAX];
    const char *data;
    int rc;

    if (!store || !did) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    if (get_file(path, 0, 4, store->root, DID_DIR, did->idstring, DOCUMENT_FILE) == -1) {
        DIDError_Set(DIDERR_NOT_EXISTS, "The file for Did document don't exist.");
        return NULL;
    }

    rc = test_path(path);
    if (rc < 0) {
        DIDError_Set(DIDERR_IO_ERROR, "File error.");
        return NULL;
    }

    if (rc == S_IFDIR) {
        DIDError_Set(DIDERR_IO_ERROR, "File error.");
        delete_file(path);
        return NULL;
    }

    data = load_file(path);
    if (!data) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Load file error.");
        return NULL;
    }

    document = DIDDocument_FromJson(data);
    free((char*)data);
    if (!document)
        return NULL;

    if (load_didmeta(store, &document->meta, document->did.idstring) == -1) {
        DIDDocument_Destroy(document);
        return NULL;
    }

    DIDMeta_SetStore(&document->meta, store);
    DIDMeta_Copy(&document->did.meta, &document->meta);

    return document;
}

bool DIDStore_ContainsDID(DIDStore *store, DID *did)
{
    char path[PATH_MAX];
    int rc;

    if (!store || !did) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    if (get_dir(path, 0, 3, store->root, DID_DIR, did->idstring) == -1) {
        DIDError_Set(DIDERR_NOT_EXISTS, "No DID [%s] in store.", did->idstring);
        return false;
    }

    rc = test_path(path);
    if (rc < 0) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Invalid DID [%s] in store.", did->idstring);
        return false;
    }

    if (rc == S_IFREG || is_empty(path)) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Invalid DID [%s] in store.", did->idstring);
        delete_file(path);
        return false;
    }

    return true;
}

bool DIDStore_DeleteDID(DIDStore *store, DID *did)
{
    char path[PATH_MAX];
    int rc;

    if (!store || !did) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    if (get_dir(path, 0, 3, store->root, DID_DIR, did->idstring) == -1) {
        DIDError_Set(DIDERR_NOT_EXISTS, "Dids don't exist.");
        return false;
    }

    if (test_path(path) > 0) {
        delete_file(path);
        return true;
    } else {
        DIDError_Set(DIDERR_IO_ERROR, "File error.");
        return false;
    }
}

int DIDStore_ListDIDs(DIDStore *store, ELA_DID_FILTER filter,
        DIDStore_DIDsCallback *callback, void *context)
{
    char path[PATH_MAX];
    DID_List_Helper dh;
    int rc;

    if (!store || !callback) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    if (get_dir(path, 0, 2, store->root, DID_DIR) == -1) {
        DIDError_Set(DIDERR_NOT_EXISTS, "The directory stored dids is not exist: %s", path);
        return -1;
    }

    rc = test_path(path);
    if (rc < 0) {
        DIDError_Set(DIDERR_IO_ERROR, "The directory stored dids error.");
        return -1;
    }

    if (rc != S_IFDIR) {
        DIDError_Set(DIDERR_IO_ERROR, "The directory stored dids should be directory.");
        return -1;
    }

    dh.store = store;
    dh.cb = callback;
    dh.context = context;
    dh.filter = filter;

    if (list_dir(path, "*", list_did_helper, (void*)&dh) == -1) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "List dids failed.");
        return -1;
    }

    return 0;
}

int DIDStore_StoreCredential(DIDStore *store, Credential *credential, const char *alias)
{
    CredentialMeta meta;
    DIDURL *id;

    if (!store || !credential) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    id = Credential_GetId(credential);
    if (!id)
        return -1;

    if (load_credmeta(store, &meta, id->did.idstring, id->fragment) == -1 ||
            CredentialMeta_SetAlias(&credential->meta, alias) == -1 ||
            CredentialMeta_Merge(&meta, &credential->meta) == -1)
        return -1;

    CredentialMeta_SetStore(&meta, store);
    memcpy(&credential->meta, &meta, sizeof(CredentialMeta));
    if (store_credential(store, credential) == -1 ||
            store_credmeta(store, &credential->meta, id) == -1)
        return -1;

    return 0;
}

Credential *DIDStore_LoadCredential(DIDStore *store, DID *did, DIDURL *id)
{
    const char *data;
    char path[PATH_MAX];
    Credential *credential;
    int rc;

    if (!store || !did ||!id)
        return NULL;

    if (get_file(path, 0, 6, store->root, DID_DIR, did->idstring,
            CREDENTIALS_DIR, id->fragment, CREDENTIAL_FILE) == -1) {
        DIDError_Set(DIDERR_NOT_EXISTS, "The file stored credential don't exist.");
        return NULL;
    }

    rc = test_path(path);
    if (rc < 0) {
        DIDError_Set(DIDERR_IO_ERROR, "The file stored credential error.");
        return NULL;
    }

    if (rc == S_IFDIR) {
        DIDError_Set(DIDERR_IO_ERROR, "The file stored credential should be a file.");
        delete_file(path);
        return NULL;
    }

    data = load_file(path);
    if (!data) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Load credential failed.");
        return NULL;
    }

    credential = Credential_FromJson(data, did);
    free((char*)data);
    if (!credential)
        return NULL;

    if (DIDStore_LoadCredMeta(store, &credential->meta, id) == -1) {
        Credential_Destroy(credential);
        return NULL;
    }
    return credential;
}

bool DIDStore_ContainsCredentials(DIDStore *store, DID *did)
{
    char path[PATH_MAX];
    int rc;
    bool empty;

    if (!store || !did) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    if (get_dir(path, 0, 4, store->root, DID_DIR, did->idstring, CREDENTIALS_DIR) == -1) {
        DIDError_Set(DIDERR_NOT_EXISTS, "The directory stored credentials is not exist: %s", path);
        return -1;
    }

    rc = test_path(path);
    if (rc < 0) {
        DIDError_Set(DIDERR_IO_ERROR, "The directory stored credentials error.");
        return false;
    }

    if (rc == S_IFREG) {
        DIDError_Set(DIDERR_IO_ERROR, "Store credentials should be directory.");
        delete_file(path);
        return false;
    }

    empty = is_empty(path);
    if (empty)
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "The directory stored credentials is empty.");

    return !empty;
}

bool DIDStore_ContainsCredential(DIDStore *store, DID *did, DIDURL *id)
{
    char path[PATH_MAX];
    int rc;

    if (!store || !did || !id) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    if (get_dir(path, 0, 5, store->root, DID_DIR, did->idstring,
            CREDENTIALS_DIR, id->fragment) == -1) {
        DIDError_Set(DIDERR_NOT_EXISTS, "Credential[%s] is not existed in did store.", id->fragment);
        return false;
    }

    rc = test_path(path);
    if (rc < 0) {
        DIDError_Set(DIDERR_IO_ERROR, "The file stored credential error.");
        return false;
    }

    if (rc == S_IFREG) {
        DIDError_Set(DIDERR_IO_ERROR, "Store credential should be directory.");
        delete_file(path);
        return false;
    }

    return true;
}

bool DIDStore_DeleteCredential(DIDStore *store, DID *did, DIDURL *id)
{
    char path[PATH_MAX];

    if (!store || !did || !id) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    if (get_dir(path, 0, 5, store->root, DID_DIR, did->idstring,
            CREDENTIALS_DIR, id->fragment) == -1) {
        DIDError_Set(DIDERR_NOT_EXISTS, "Credential[%s] is not existed in did store.", id->fragment);
        return false;
    }

    if (is_empty(path)) {
        DIDError_Set(DIDERR_IO_ERROR, "The directory stored credential is empty.");
        return false;
    }

    delete_file(path);
    if (get_dir(path, 0, 4, store->root, DID_DIR, did->idstring, CREDENTIALS_DIR) == 0) {
        if (is_empty(path))
            delete_file(path);
    }
    return true;
}

int DIDStore_ListCredentials(DIDStore *store, DID *did,
        DIDStore_CredentialsCallback *callback, void *context)
{
    ssize_t size = 0;
    char path[PATH_MAX];
    Cred_List_Helper ch;
    int rc;

    if (!store || !did || !callback) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    if (get_dir(path, 0, 4, store->root, DID_DIR, did->idstring, CREDENTIALS_DIR) == -1) {
        DIDError_Set(DIDERR_NOT_EXISTS, "The directory stored credentials is not exist: %s", path);
        return -1;
    }

    rc = test_path(path);
    if (rc < 0) {
        DIDError_Set(DIDERR_IO_ERROR, "The directory stored credentials error.");
        return -1;
    }

    if (rc == S_IFREG) {
        DIDError_Set(DIDERR_IO_ERROR, "Store credentials should be directory.");
        delete_file(path);
        return -1;
    }

    ch.store = store;
    ch.cb = callback;
    ch.context = context;
    strcpy((char*)ch.did.idstring, did->idstring);
    ch.type = NULL;

    if (list_dir(path, "*", list_credential_helper, (void*)&ch) == -1) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "List credentials failed.");
        return -1;
    }

    return 0;
}

int DIDStore_SelectCredentials(DIDStore *store, DID *did, DIDURL *id,
        const char *type, DIDStore_CredentialsCallback *callback, void *context)
{
    char path[PATH_MAX];
    Cred_List_Helper ch;
    int rc;

    if (!store || !did || !callback) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }
    if (!id && !type) {
        DIDError_Set(DIDERR_INVALID_ARGS, "No feature to select credential.");
        return -1;
    }

    if (id) {
        if (get_file(path, 0, 6, store->root, DID_DIR, did->idstring,
                CREDENTIALS_DIR, id->fragment, CREDENTIAL_FILE) == -1) {
            DIDError_Set(DIDERR_NOT_EXISTS, "Credential don't exist.");
            return -1;
        }

        if (test_path(path) > 0) {
            if ((type && has_type(did, path, type) == true) || !type) {
                if (callback(id, context) < 0) {
                    DIDError_Set(DIDERR_DIDSTORE_ERROR, "Select credential callback error.");
                }

                return 0;
            }

            DIDError_Set(DIDERR_DIDSTORE_ERROR, "No credential is match with type.");
            return -1;
        }

        DIDError_Set(DIDERR_UNKNOWN, "Unknown error.");
        return -1;
    }

    if (get_dir(path, 0, 4, store->root, DID_DIR, did->idstring, CREDENTIALS_DIR) == -1) {
        DIDError_Set(DIDERR_NOT_EXISTS, "No credentials.");
        return -1;
    }

    rc = test_path(path);
    if (rc < 0) {
        DIDError_Set(DIDERR_IO_ERROR, "The file for credential error.");
        return -1;
    }

    if (rc == S_IFREG) {
        DIDError_Set(DIDERR_IO_ERROR, "Credential should be directory.");
        delete_file(path);
        return -1;
    }

    ch.store = store;
    ch.cb = callback;
    ch.context = context;
    strcpy((char*)ch.did.idstring, did->idstring);
    ch.type = type;

    if (list_dir(path, "*.*", select_credential_helper, (void*)&ch) == -1) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "List credentials failed.");
        return -1;
    }

    return 0;
}

bool DIDSotre_ContainsPrivateKeys(DIDStore *store, DID *did)
{
    char path[PATH_MAX];
    bool empty;

    if (!store || !did) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    if (get_dir(path, 0, 4, store->root, DID_DIR, did->idstring, PRIVATEKEYS_DIR) == -1) {
        DIDError_Set(DIDERR_NOT_EXISTS, "The directory stored private keys is not exist: %s", path);
        return -1;
    }

    empty = is_empty(path);
    if (empty)
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "The directory stored private keys is empty.");

    return !empty;
}

bool DIDStore_ContainsPrivateKey(DIDStore *store, DID *did, DIDURL *id)
{
    char path[PATH_MAX];
    int rc;

    if (!store || !did || !id) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    if (get_file(path, 0, 5, store->root, DID_DIR, did->idstring,
            PRIVATEKEYS_DIR, id->fragment) == -1) {
        DIDError_Set(DIDERR_NOT_EXISTS, "The file for private key don't exist.");
        return false;
    }

    rc = test_path(path);
    if (rc < 0) {
        DIDError_Set(DIDERR_IO_ERROR, "Private key file error.");
        return false;
    }

    if (rc == S_IFDIR) {
        DIDError_Set(DIDERR_IO_ERROR, "The file for private key should be file.");
        delete_file(path);
        return false;
    }

    return true;
}

int DIDStore_StorePrivateKey(DIDStore *store, const char *storepass, DID *did,
        DIDURL *id, const uint8_t *privatekey)
{
    char path[PATH_MAX];
    char base64[MAX_PRIVATEKEY_BASE64];

    if (!store || !storepass || !*storepass || !did || !id || !privatekey) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    if (!DID_Equals(DIDURL_GetDid(id), did)) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Private key does not match with did.");
        return -1;
    }

    if (encrypt_to_base64(base64, storepass, privatekey, PRIVATEKEY_BYTES) == -1) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Encrypt private key failed.");
        return -1;
    }

    if (get_file(path, 1, 5, store->root, DID_DIR, did->idstring,
            PRIVATEKEYS_DIR, id->fragment) == -1) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Create private key file failed.");
        return -1;
    }

    if (!store_file(path, base64))
        return 0;

    DIDError_Set(DIDERR_DIDSTORE_ERROR, "Store private key error: %s", path);
    delete_file(path);
    return -1;
}

void DIDStore_DeletePrivateKey(DIDStore *store, DID *did, DIDURL *id)
{
    char path[PATH_MAX];

    if (!store || !did || !id)
        return;

    if (get_file(path, 0, 6, store->root, DID_DIR, did->idstring,
            PRIVATEKEYS_DIR, id->fragment) == -1)
        return;

    if (test_path(path) > 0)
        delete_file(path);

    return;
}

//caller provide HDKey object.
static HDKey *load_privateIdentity(DIDStore *store, const char *storepass,
        HDKey *identity)
{
    uint8_t extendedkey[EXTENDEDKEY_BYTES];
    ssize_t size;
    int rc = 0;

    assert(store);
    assert(storepass && *storepass);
    assert(identity);

    size = load_extendedprvkey(store, extendedkey, sizeof(extendedkey), storepass);
    if (size == -1)
        return NULL;

    if (size == SEED_BYTES) {
        if (HDKey_FromSeed(extendedkey, size, identity)) {
            size = HDKey_SerializePrv(identity, extendedkey, sizeof(extendedkey));
            if (size != -1) {
                rc = store_extendedprvkey(store, extendedkey, size, storepass);
            }
        }
    } else {
        if (!HDKey_FromExtendedKey(extendedkey, sizeof(extendedkey), identity)) {
            DIDError_Set(DIDERR_CRYPTO_ERROR, "Initial private identity failed.");
            rc = -1;
        }
    }

    memset(extendedkey, 0, sizeof(extendedkey));
    return rc == 0 ? identity : NULL;
}

static HDKey *load_publicIdentity(DIDStore *store, HDKey *identity)
{
    uint8_t extendedkey[EXTENDEDKEY_BYTES];
    ssize_t size;
    HDKey *hdkey;

    assert(store);
    assert(identity);

    size = load_extendedpubkey(store, extendedkey, sizeof(extendedkey));
    if (size == -1)
        return NULL;

    hdkey = HDKey_FromExtendedKey(extendedkey, sizeof(extendedkey), identity);
    if (!hdkey)
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Initial private identity failed.");

    return hdkey;
}

static int store_default_privatekey(DIDStore *store, const char *storepass,
        const char *idstring, uint8_t *privatekey)
{
    DID did;
    DIDURL id;
    int rc;

    assert(store);
    assert(storepass && *storepass);
    assert(idstring && *idstring);
    assert(privatekey);

    if (Init_DID(&did, idstring) == -1 || Init_DIDURL(&id, &did, "primary") == -1)
        return -1;

    if (DIDStore_StorePrivateKey(store, storepass, &did, &id, (unsigned char *)privatekey) == -1)
        return -1;

    return 0;
}

int DIDStore_Synchronize(DIDStore *store, const char *storepass, DIDStore_MergeCallback *callback)
{
    int rc, nextindex, i = 0, blanks = 0;
    DIDDocument *chainCopy, *localCopy, *finalCopy;
    HDKey _identity, *privateIdentity;
    DerivedKey _derivedkey, *derivedkey;
    DID did;

     if (!store || !storepass || !*storepass || !callback)
        return -1;

    privateIdentity = load_privateIdentity(store, storepass, &_identity);
    if (!privateIdentity)
        return -1;

    nextindex = load_index(store);
    if (nextindex < 0) {
        HDKey_Wipe(privateIdentity);
        return -1;
    }

    while (i < nextindex || blanks < 20) {
        derivedkey = HDKey_GetDerivedKey(privateIdentity, i++, &_derivedkey);
        if (!derivedkey)
            continue;

        if (Init_DID(&did, DerivedKey_GetAddress(derivedkey)) == 0) {
            chainCopy = DID_Resolve(&did, true);
            if (chainCopy) {
                if (DIDDocument_IsDeactivated(chainCopy)) {
                    DIDError_Set(DIDERR_DID_DEACTIVATED, "Did is deactivated.");
                    DIDDocument_Destroy(chainCopy);
                    blanks = 0;
                    continue;
                }

                if (DIDDocument_IsExpires(chainCopy)) {
                    DIDError_Set(DIDERR_EXPIRED, "Did is expired.");
                    DIDDocument_Destroy(chainCopy);
                    blanks = 0;
                    continue;
                }

                finalCopy = chainCopy;
                localCopy = DIDStore_LoadDID(store, &did);
                if (localCopy) {
                    const char *local_signature = DIDMeta_GetSignature(&localCopy->meta);
                    if (!*local_signature ||
                            strcmp(DIDDocument_GetProofSignature(localCopy), local_signature)) {
                        finalCopy = callback(chainCopy, localCopy);
                        if (!finalCopy || !DID_Equals(DIDDocument_GetSubject(finalCopy), &did)) {
                            DIDError_Set(DIDERR_DIDSTORE_ERROR, "Conflict handle merge the DIDDocument error.");
                            if (finalCopy != chainCopy && finalCopy != localCopy)
                                DIDDocument_Destroy(finalCopy);
                            DIDDocument_Destroy(chainCopy);
                            DIDDocument_Destroy(localCopy);
                            return -1;
                        }
                    }
                }

                if (DIDStore_StoreDID(store, finalCopy, NULL) == 0) {
                    if (store_default_privatekey(store, storepass,
                            DerivedKey_GetAddress(derivedkey),
                            DerivedKey_GetPrivateKey(derivedkey)) == 0) {
                        if (store_index(store, i) == -1)
                            DIDStore_DeleteDID(store, &did);

                    } else {
                        DIDStore_DeleteDID(store, &did);
                    }
                }
                if (finalCopy != chainCopy && finalCopy != localCopy)
                    DIDDocument_Destroy(finalCopy);

                DIDDocument_Destroy(chainCopy);
                DIDDocument_Destroy(localCopy);
                blanks = 0;
            } else {
                if (i >= nextindex)
                    blanks++;
            }
        }
        DerivedKey_Wipe(derivedkey);
    }

    HDKey_Wipe(privateIdentity);
    return 0;
}

bool DIDStore_ContainsPrivateIdentity(DIDStore *store)
{
    const char *seed;
    char path[PATH_MAX];
    struct stat st;

    if (!store) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    if (get_file(path, 0, 3, store->root, PRIVATE_DIR, HDKEY_FILE) == -1) {
        DIDError_Set(DIDERR_NOT_EXISTS, "Private identity don't already exist.");
        return false;
    }

    if (stat(path, &st) < 0) {
        DIDError_Set(DIDERR_IO_ERROR, "File error.");
        return false;
    }

    if (st.st_size <= 0) {
        DIDError_Set(DIDERR_IO_ERROR, "No private identity.");
        return false;
    }

    return true;
}

int DIDStore_InitPrivateIdentity(DIDStore *store, const char *storepass,
        const char *mnemonic, const char *passphrase, const char *language, bool force)
{
    uint8_t extendedkey[EXTENDEDKEY_BYTES];
    char path[PATH_MAX];
    ssize_t size;
    HDKey _privateIdentity, *privateIdentity;

    if (!store || !storepass || !*storepass|| !mnemonic || !*mnemonic) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    if (!passphrase)
        passphrase = "";

    //check if DIDStore has existed private identity
    if (get_file(path, 0, 3, store->root, PRIVATE_DIR, HDKEY_FILE) == 0) {
        if (DIDStore_ContainsPrivateIdentity(store) && !force) {
            DIDError_Set(DIDERR_ALREADY_EXISTS, "Private identity don't exist.");
            return -1;
        }
    }

    privateIdentity = HDKey_FromMnemonic(mnemonic, passphrase, language, &_privateIdentity);
    if (!privateIdentity) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Initial private identity failed.");
        return -1;
    }

    size = HDKey_SerializePrv(privateIdentity, extendedkey, sizeof(extendedkey));
    if (size == -1) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Serialize extended private key failed.");
        return -1;
    }

    if (store_extendedprvkey(store, extendedkey, size, storepass) == -1)
        return -1;

    memset(extendedkey, 0, sizeof(extendedkey));
    size = HDKey_SerializePub(privateIdentity, extendedkey, sizeof(extendedkey));
    if (size == -1) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Serialize extended public key failed.");
        return -1;
    }

    if (store_extendedpubkey(store, extendedkey, size) == -1)
        return -1;

    if (store_mnemonic(store, storepass, mnemonic) == -1)
        return -1;

    if (store_index(store, 0) == -1)
        return -1;

    return 0;
}

int DIDStore_InitPrivateIdentityFromRootKey(DIDStore *store, const char *storepass,
        const char *extendedprvkey, bool force)
{
    uint8_t seed[SEED_BYTES], extendedkey[EXTENDEDKEY_BYTES];
    char path[PATH_MAX];
    ssize_t size;
    HDKey _privateIdentity, *privateIdentity;

    if (!store || !storepass || !*storepass || !extendedprvkey || !*extendedprvkey) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    //check if DIDStore has existed private identity
    if (get_file(path, 0, 3, store->root, PRIVATE_DIR, HDKEY_FILE) == 0) {
        if (DIDStore_ContainsPrivateIdentity(store) && !force) {
            DIDError_Set(DIDERR_ALREADY_EXISTS, "Private identity is already exist.");
            return -1;
        }
    }

    size = base58_decode(extendedkey, sizeof(extendedkey), extendedprvkey);
    if (size == -1) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Decode extended private key failed.");
        return -1;
    }

    privateIdentity = HDKey_FromExtendedKey(extendedkey, size, &_privateIdentity);
    if (!privateIdentity) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Initial private identity failed.");
        return -1;
    }

    size = HDKey_SerializePrv(privateIdentity, extendedkey, sizeof(extendedkey));
    if (size == -1) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Serialize extended private key failed.");
        return -1;
    }

    if (store_extendedprvkey(store, extendedkey, size, storepass) == -1)
        return -1;

    memset(extendedkey, 0, sizeof(extendedkey));
    size = HDKey_SerializePub(privateIdentity, extendedkey, sizeof(extendedkey));
    if (size == -1) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Serialize extended public key failed.");
        return -1;
    }

    if (store_extendedpubkey(store, extendedkey, size) == -1)
        return -1;

    if (store_index(store, 0) == -1)
        return -1;

    return 0;
}

static DerivedKey *get_childkey(DIDStore *store, const char *storepass, int index,
        DerivedKey *derivedkey)
{
    HDKey _identity, *identity;
    DerivedKey *dkey;

    assert(store);
    assert(index >= 0);
    assert(derivedkey);

    if (!storepass)
        identity = load_publicIdentity(store, &_identity);
    else
        identity = load_privateIdentity(store, storepass, &_identity);

    if (!identity)
        return NULL;

    dkey = HDKey_GetDerivedKey(identity, index, derivedkey);
    HDKey_Wipe(identity);
    if (!dkey)
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Initial derived private identity failed.");

    return dkey;
}

DIDDocument *DIDStore_NewDID(DIDStore *store, const char *storepass, const char *alias)
{
    int index;
    DIDDocument *document;

    if (!store || !storepass || !*storepass) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    index = load_index(store);
    if (index < 0)
        return NULL;

    document = DIDStore_NewDIDByIndex(store, storepass, index++, alias);
    if (!document)
        return NULL;

    if (store_index(store, index) == -1) {
        DIDStore_DeleteDID(store, DIDDocument_GetSubject(document));
        DIDDocument_Destroy(document);
        return NULL;
    }

    return document;
}

//free DID after use it.
DID *DIDStore_GetDIDByIndex(DIDStore *store, int index)
{
    DerivedKey _derivedkey, *derivedkey;
    DID *did;
    int rc;

    if (!store || index < 0) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    derivedkey = get_childkey(store, NULL, index, &_derivedkey);
    if (!derivedkey)
        return NULL;

    did = DID_New(DerivedKey_GetAddress(derivedkey));
    DerivedKey_Wipe(derivedkey);
    return did;
}

DIDDocument *DIDStore_NewDIDByIndex(DIDStore *store, const char *storepass,
        int index, const char *alias)
{
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    DerivedKey _derivedkey, *derivedkey;
    DIDDocument *document;
    DID did;

    if (!store || !storepass || !*storepass || index < 0) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    derivedkey = get_childkey(store, storepass, index, &_derivedkey);
    if (!derivedkey)
        return NULL;

    if (Init_DID(&did, DerivedKey_GetAddress(derivedkey)) == -1) {
        DerivedKey_Wipe(derivedkey);
        return NULL;
    }

    //check did is exist or not
    document = DIDStore_LoadDID(store, &did);
    if (document) {
        DerivedKey_Wipe(derivedkey);
        DIDDocument_Destroy(document);
        return NULL;
    }

    if (store_default_privatekey(store, storepass,
            DerivedKey_GetAddress(derivedkey),
            DerivedKey_GetPrivateKey(derivedkey)) == -1) {
        DerivedKey_Wipe(derivedkey);
        return NULL;
    }

    base58_encode(publickeybase58, sizeof(publickeybase58),
            DerivedKey_GetPublicKey(derivedkey), PUBLICKEY_BYTES);
    DerivedKey_Wipe(derivedkey);

    document = create_document(store, &did, publickeybase58, storepass, alias);
    if (!document) {
        DIDStore_DeleteDID(store, &did);
        return NULL;
    }

    if (DIDStore_StoreDID(store, document, alias) == -1) {
        DIDStore_DeleteDID(store, &did);
        DIDDocument_Destroy(document);
        return NULL;
    }

    DIDDocument_SetStore(document, store);
    return document;
}

int DIDStore_LoadPrivateKey(DIDStore *store, const char *storepass, DID *did,
        DIDURL *key, uint8_t *privatekey)
{
    const char *privatekey_str;
    char path[PATH_MAX];
    int rc;

    assert(store);
    assert(storepass && *storepass);
    assert(did);
    assert(key);
    assert(privatekey);

    if (get_file(path, 0, 5, store->root, DID_DIR, did->idstring, PRIVATEKEYS_DIR, key->fragment) == -1) {
        DIDError_Set(DIDERR_NOT_EXISTS, "No private key file.");
        return -1;
    }

    privatekey_str = load_file(path);
    if (!privatekey_str) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "No valid private key.");
        return -1;
    }

    rc = decrypt_from_base64(privatekey, storepass, privatekey_str);
    free((char*)privatekey_str);
    if (rc == -1) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Decrypt private key failed.");
        return -1;
    }

    return 0;
}

int DIDStore_Sign(DIDStore *store, const char *storepass, DID *did,
        DIDURL *key, char *sig, uint8_t *digest, size_t size)
{
    uint8_t binkey[PRIVATEKEY_BYTES];

    if (!store || !storepass || !*storepass || !did || !key
            || !sig || !digest || size != SHA256_BYTES) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    if (DIDStore_LoadPrivateKey(store, storepass, did, key, binkey) == -1)
        return -1;

    if (ecdsa_sign_base64(sig, binkey, digest, size) == -1) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "ECDSA sign failed.");
        return -1;
    }

    memset(binkey, 0, sizeof(binkey));
    return 0;
}

const char *DIDStore_PublishDID(DIDStore *store, const char *storepass, DID *did,
        DIDURL *signkey, bool force)
{
    const char *local_txid, *last_txid;
    const char *local_signature, *resolve_signature;
    DIDDocument *doc = NULL, *resolve_doc = NULL;
    int rc = -1;

    if (!store || !storepass || !*storepass || !did) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    doc = DIDStore_LoadDID(store, did);
    if (!doc)
        return NULL;

    if (DIDDocument_IsDeactivated(doc)) {
        DIDError_Set(DIDERR_DID_DEACTIVATED, "Did is already deactivated.");
        goto errorExit;
    }

    if (!force && DIDDocument_IsExpires(doc)) {
        DIDError_Set(DIDERR_EXPIRED, "Did already expired, use force mode to publish anyway.");
        goto errorExit;
    }

    if (!signkey)
        signkey = DIDDocument_GetDefaultPublicKey(doc);

    resolve_doc = DID_Resolve(did, true);
    if (!resolve_doc) {
        last_txid = DIDBackend_Create(&store->backend, doc, signkey, storepass);
    } else {
        if (DIDDocument_IsDeactivated(resolve_doc)) {
            DIDError_Set(DIDERR_EXPIRED, "Did already deactivated.");
            goto errorExit;
        }

        last_txid = DIDDocument_GetTxid(resolve_doc);
        if (!last_txid || !*last_txid) {
            DIDError_Set(DIDERR_RESOLVE_ERROR, "Missing last transaction id.");
            goto errorExit;
        }

        if (!force) {
            local_txid = DIDMeta_GetTxid(&doc->meta);
            local_signature = DIDMeta_GetSignature(&doc->meta);
            if ((!local_txid || !*local_txid) && (!local_signature || !*local_signature)) {
                DIDError_Set(DIDERR_DIDSTORE_ERROR,
                        "Missing last transaction id and signature, use force mode to ignore checks.");
                goto errorExit;
            }

            resolve_signature = DIDDocument_GetProofSignature(resolve_doc);
            if (!resolve_signature || !*resolve_signature) {
                DIDError_Set(DIDERR_DIDSTORE_ERROR, "Missing transaction id and signature on chain.");
                goto errorExit;
            }

            if ((*local_txid && strcmp(local_txid, last_txid)) ||
                    (*local_signature && strcmp(local_signature, resolve_signature))) {
                DIDError_Set(DIDERR_DIDSTORE_ERROR,
                        "Current copy not based on the lastest on-chain copy, txid mismatch.");
                goto errorExit;
            }
        }

        DIDMeta_SetTxid(&doc->did.meta, last_txid);
        DIDMeta_SetTxid(&doc->meta, last_txid);

        last_txid = DIDBackend_Update(&store->backend, doc, signkey, storepass);
    }

    if (!last_txid || !*last_txid)
        goto errorExit;

    DIDMeta_SetTxid(&doc->meta, last_txid);
    DIDMeta_SetSignature(&doc->meta, DIDDocument_GetProofSignature(doc));
    rc = store_didmeta(store, &doc->meta, &doc->did);

errorExit:
    if (resolve_doc)
        DIDDocument_Destroy(resolve_doc);
    if (doc)
        DIDDocument_Destroy(doc);
    return rc == -1 ? NULL : last_txid;
}

const char *DIDStore_DeactivateDID(DIDStore *store, const char *storepass,
        DID *did, DIDURL *signkey)
{
    const char *txid;
    DIDDocument *doc;
    bool localcopy = false;
    int rc = 0;

    if (!store || !storepass || !*storepass || !did) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    doc = DID_Resolve(did, true);
    if (!doc) {
        doc = DIDStore_LoadDID(store, did);
        if (!doc) {
            DIDError_Set(DIDERR_NOT_EXISTS, "No this did.");
            return NULL;
        } else {
            localcopy = true;
        }
    }
    else {
        DIDMeta_SetStore(&doc->meta, store);
        DIDMeta_SetStore(&doc->did.meta, store);
    }

    if (!signkey) {
        signkey = DIDDocument_GetDefaultPublicKey(doc);
        if (!signkey) {
            DIDDocument_Destroy(doc);
            DIDError_Set(DIDERR_INVALID_KEY, "Not default key.");
            return NULL;
        }
    } else {
        if (!DIDDocument_IsAuthenticationKey(doc, signkey)) {
            DIDDocument_Destroy(doc);
            DIDError_Set(DIDERR_INVALID_KEY, "Invalid authentication key.");
            return NULL;
        }
    }

    txid = DIDBackend_Deactivate(&store->backend, did, signkey, storepass);

    if (localcopy) {
        DIDMeta_SetDeactived(&doc->meta, true);
        rc = store_didmeta(store, &doc->meta, &doc->did);
    }

    DIDDocument_Destroy(doc);
    return rc == 0 ? txid : NULL;
}

static bool need_reencrypt(const char *path)
{
    char file[PATH_MAX];
    char *token;
    int i = 0;

    assert(path && *path);

    strcpy(file, path);

    if (!strcmp(file + strlen(file) - strlen(KEY_PATH), KEY_PATH) ||
            !strcmp(file + strlen(file) - strlen(MNEMONIC_PATH), MNEMONIC_PATH))
        return true;

    //handle did privatekeys
    token = strtok((char*)file, PATH_SEP);
    while(token) {
        if (i >= 1 || !strcmp(token, DID_DIR))
            i++;
        if (i == 3 && !strcmp(token, PRIVATEKEYS_DIR))
            i++;
        if (i == 4 && strlen(token) > 0)
            return true;
        token = strtok(NULL, PATH_SEP);
    }

    return false;
}

int dir_copy(const char *dst, const char *src, const char *new, const char *old);

static int dir_copy_helper(const char *path, void *context)
{
    char srcpath[PATH_MAX];
    char dstpath[PATH_MAX];
    int rc;

    Dir_Copy_Helper *dh = (Dir_Copy_Helper*)context;

    if (!path)
        return 0;

    if (strcmp(path, ".") == 0 || strcmp(path, "..") == 0)
        return 0;

    sprintf(srcpath, "%s/%s", dh->srcpath, path);
    sprintf(dstpath, "%s/%s", dh->dstpath, path);

    return dir_copy(dstpath, srcpath, dh->newpassword, dh->oldpassword);
}

int dir_copy(const char *dst, const char *src, const char *new, const char *old)
{
    int rc;
    Dir_Copy_Helper dh;
    const char *string;
    ssize_t size;
    uint8_t plain[256];
    unsigned char data[512];

    assert(dst && *dst);
    assert(src && *src);

    if (test_path(src) < 0)
        return -1;

    //src is directory.
    if (test_path(src) == S_IFDIR) {
        if (test_path(dst) < 0) {
            rc = mkdirs(dst, S_IRWXU);
            if (rc < 0) {
                DIDError_Set(DIDERR_IO_ERROR, "Create cache directory (%s) failed", dst);
                return -1;
            }
        }

        dh.srcpath = src;
        dh.dstpath = dst;
        dh.oldpassword = old;
        dh.newpassword = new;

        if (list_dir(src, "*", dir_copy_helper, (void*)&dh) == -1) {
            DIDError_Set(DIDERR_DIDSTORE_ERROR, "Copy directory failed.");
            return -1;
        }

        return 0;
    }

    //src is file
    string = load_file(src);
    if (!string || !*string) {
        DIDError_Set(DIDERR_IO_ERROR, "Load %s failed.", src);
        return -1;
    }

    //src is not encrypted file.
    if (!need_reencrypt(src)) {
        rc = store_file(dst, string);
        free((char*)string);
        if (rc < 0)
            DIDError_Set(DIDERR_IO_ERROR, "Store %s failed.", dst);
        return rc;
    }

    //src is encrypted file.
    size = decrypt_from_base64(plain, old, string);
    free((char*)string);
    if (size < 0) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Decrypt %s failed.", src);
        return -1;
    }

    size = encrypt_to_base64((char*)data, new, plain, size);
    memset(plain, 0, sizeof(plain));
    if (size < 0) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Encrypt %s with new pass word failed.", src);
        return -1;
    }

    rc = store_file(dst, (char*)data);
    if (rc < 0)
        DIDError_Set(DIDERR_IO_ERROR, "Store %s failed.", dst);

    return rc;
}

static int changepassword(DIDStore *store, const char *new, const char *old)
{
    char private_dir[PATH_MAX] = {0}, private_journal[PATH_MAX]= {0};
    char did_dir[PATH_MAX]= {0}, did_journal[PATH_MAX]= {0};
    char path[PATH_MAX] = {0};

    assert(store);
    assert(new && *new);
    assert(old && *old);

    if (get_dir(private_dir, 0, 2, store->root, PRIVATE_DIR) == -1) {
        DIDError_Set(DIDERR_NOT_EXISTS, "Private identity doesn't exist.");
        return -1;
    }
    if (test_path(private_dir) != S_IFDIR) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Private identity is not a directory.");
        return -1;
    }

    if (get_dir(private_journal, 1, 2, store->root, PRIVATE_JOURNAL) == -1) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Create private identiaty journal failed.");
        return -1;
    }
    if (test_path(private_journal) != S_IFDIR) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Private identity journal is not a directory.");
        return -1;
    }

    if (get_dir(did_dir, 0, 2, store->root, DID_DIR) == -1) {
        DIDError_Set(DIDERR_NOT_EXISTS, "DID directory doesn't exist.");
        return -1;
    }
    if (test_path(did_dir) != S_IFDIR) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "DID directory is not a directory.");
        return -1;
    }

    if (get_dir(did_journal, 1, 2, store->root, DID_JOURNAL) == -1) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Create did journal failed.");
        return -1;
    }
    if (test_path(did_journal) != S_IFDIR) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "DID journal is not a directory.");
        return -1;
    }

    if (dir_copy(private_journal, private_dir, new, old) == -1) {
        delete_file(private_journal);
        return -1;
    }

    if (dir_copy(did_journal, did_dir, new, old) == -1) {
        delete_file(private_journal);
        delete_file(did_journal);
        return -1;
    }

    //create tag file to indicate copying dir successfully.
    if (get_file(path, 1, 2, store->root, POST_PASSWORD) == -1) {
        delete_file(private_journal);
        delete_file(did_journal);
        return -1;
    }

    return store_file(path, "");
}

int DIDStore_ChangePassword(DIDStore *store, const char *new, const char *old)
{
    if (!store || !old || !*old || !new || !*new)
        return -1;

    if (changepassword(store, new, old) == -1)
        return -1;

    return postChangePassword(store);
}
