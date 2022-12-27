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
#include <zip.h>

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
#include "HDkey.h"
#include "resolvercache.h"

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

static const char *DID_EXPORT = "did.elastos.export/1.0";

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

typedef struct Cred_Export_Helper {
    DIDStore *store;
    JsonGenerator *gen;
    Sha256_Digest *digest;
} Cred_Export_Helper;

typedef struct Prvkey_Export {
    DIDURL keyid;
    char key[512];
} Prvkey_Export;

typedef struct DID_Export {
    DIDStore *store;
    const char *storepass;
    const char *password;
    const char *tmpdir;
    zip_t *zip;
} DID_Export;

static int store_didmeta(DIDStore *store, DIDMetaData *meta, DID *did)
{
    char path[PATH_MAX];
    const char *data;
    int rc;

    assert(store);
    assert(meta);
    assert(did);

    if (get_file(path, 1, 4, store->root, DID_DIR, did->idstring, META_FILE) == -1) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Create file for didmeta file failed.");
        return -1;
    }

    if (test_path(path) == S_IFDIR) {
        DIDError_Set(DIDERR_IO_ERROR, "Did meta should be a file.");
        delete_file(path);
        return -1;
    }

    data = DIDMetaData_ToJson(meta);
    if (!data) {
        delete_file(path);
        return -1;
    }

    rc = store_file(path, data);
    free((void*)data);
    if (rc)
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Store did meta failed.");

    return rc;
}

static int load_didmeta(DIDStore *store, DIDMetaData *meta, const char *did)
{
    const char *data;
    char path[PATH_MAX];
    int rc;
    time_t lastmodified;

    assert(store);
    assert(meta);
    assert(did);

    memset(meta, 0, sizeof(DIDMetaData));
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

    rc = DIDMetaData_FromJson(meta, data);
    free((void*)data);
    if (rc < 0)
        return -1;

    //set last modified
    if (get_file(path, 0, 4, store->root, DID_DIR, did, DOCUMENT_FILE) == -1) {
        DIDMetaData_Free(meta);
        DIDError_Set(DIDERR_NOT_EXISTS, "The file for Did document don't exist.");
        return -1;
    }

    lastmodified = get_file_lastmodified(path);
    if (lastmodified == 0) {
        DIDMetaData_Free(meta);
        DIDError_Set(DIDERR_IO_ERROR, "Get last modified of document failed.");
        return -1;
    }

    DIDMetaData_SetLastModified(meta, lastmodified);
    DIDMetaData_SetStore(meta, store);
    return 0;
}

int DIDStore_LoadDIDMeta(DIDStore *store, DIDMetaData *meta, DID *did)
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

int DIDStore_StoreDIDMetaData(DIDStore *store, DIDMetaData *meta, DID *did)
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

static int store_credmeta(DIDStore *store, CredentialMetaData *meta, DIDURL *id)
{
    char path[PATH_MAX];
    const char *data;
    int rc;

    assert(store);
    assert(meta);
    assert(id);

    if (!meta->base.data)
        return 0;

    data = CredentialMetaData_ToJson(meta);
    if (!data)
        return -1;

    if (get_file(path, 1, 6, store->root, DID_DIR, id->did.idstring,
            CREDENTIALS_DIR, id->fragment, META_FILE) == -1) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Create file for credential meta failed.");
        free((void*)data);
        return -1;
    }

    if (test_path(path) == S_IFDIR) {
        DIDError_Set(DIDERR_IO_ERROR, "Credential meta should be a file.");
        free((void*)data);
        goto errorExit;
    }

    rc = store_file(path, data);
    free((void*)data);
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

static int load_credmeta(DIDStore *store, CredentialMetaData *meta, const char *did,
        const char *fragment)
{
    const char *data;
    char path[PATH_MAX];
    time_t lastmodified;
    int rc;

    assert(store);
    assert(meta);
    assert(did);
    assert(fragment);

    memset(meta, 0, sizeof(CredentialMetaData));
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

    rc = CredentialMetaData_FromJson(meta, data);
    free((void*)data);
    if (rc < 0)
        return -1;

    //set last modified
    if (get_file(path, 0, 6, store->root, DID_DIR, did, CREDENTIALS_DIR,
            fragment, CREDENTIAL_FILE) == -1) {
        CredentialMetaData_Free(meta);
        DIDError_Set(DIDERR_NOT_EXISTS, "Credential don't exist.");
        return -1;
    }

    lastmodified = get_file_lastmodified(path);
    if (lastmodified == 0) {
        CredentialMetaData_Free(meta);
        DIDError_Set(DIDERR_IO_ERROR, "Get last modified of document failed.");
        return -1;
    }

    CredentialMetaData_SetLastModified(meta, lastmodified);
    CredentialMetaData_SetStore(meta, store);
    return 0;
}

int DIDStore_StoreCredMeta(DIDStore *store, CredentialMetaData *meta, DIDURL *id)
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

int DIDStore_LoadCredMeta(DIDStore *store, CredentialMetaData *meta, DIDURL *id)
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

static int postchange_password(DIDStore *store)
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

    return postchange_password(store);
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

    memset(base64, 0, sizeof(base64));
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
    free((void*)string);
    if (len < 0)
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Decrypt extended private key failed.");

    return len;
}

static int store_pubkey(DIDStore *store, const char *keybase58)
{
    char path[PATH_MAX];

    assert(store);
    assert(keybase58);

    if (get_file(path, 1, 3, store->root, PRIVATE_DIR, HDPUBKEY_FILE) == -1) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Create file for extended public key failed.");
        return -1;
    }

    if (store_file(path, (const char *)keybase58) == -1) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Store extended public key failed.");
        return -1;
    }
    return 0;
}

static int store_extendedpubkey(DIDStore *store, uint8_t *extendedkey, size_t size)
{
    char publickeybase58[MAX_PUBKEY_BASE58];

    assert(store);
    assert(extendedkey && size > 0);

    if (base58_encode(publickeybase58, sizeof(publickeybase58), extendedkey, size) == -1) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Decode extended public key failed.");
        return -1;
    }

    return store_pubkey(store, publickeybase58);
}

static const char *load_pubkey_file(DIDStore *store)
{
    const char *string;
    char path[PATH_MAX];

    assert(store);

    if (get_file(path, 0, 3, store->root, PRIVATE_DIR, HDPUBKEY_FILE) == -1) {
        DIDError_Set(DIDERR_NOT_EXISTS, "Extended public key don't exist.");
        return NULL;
    }

    string = load_file(path);
    if (!string) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Load extended public key failed.");
        return NULL;
    }

    return string;
}

static ssize_t load_extendedpubkey(DIDStore *store, uint8_t *extendedkey, size_t size)
{
    const char *string;
    ssize_t len;

    assert(store);
    assert(extendedkey && size >= EXTENDEDKEY_BYTES);

    string = load_pubkey_file(store);
    if (!string)
        return -1;

    len = base58_decode(extendedkey, size, string);
    free((void*)string);
    if (len < 0)
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Decode extended public key failed.");

    return len;
}

static int store_mnemonic(DIDStore *store, const char *storepass, const uint8_t *mnemonic,
        size_t size)
{
    char base64[512];
    char path[PATH_MAX];

    assert(store);
    assert(mnemonic);
    assert(size > 0);

    memset(base64, 0, sizeof(base64));
    if (encrypt_to_base64(base64, storepass, mnemonic, size) == -1) {
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
    free((void*)encrpted_mnemonic);
    if (len < 0)
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Decrypt mnemonic failed.");

    mnemonic[len++] = 0;

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
    free((void*)string);
    return index;
}

static int store_index_string(DIDStore *store, const char *index)
{
    char path[PATH_MAX];

    assert(store);
    assert(index && *index);

    if (get_file(path, 1, 3, store->root, PRIVATE_DIR, INDEX_FILE) == -1) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Create file for index failed.");
        return -1;
    }

    if (store_file(path, index) == -1) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Store index failed.");
        return -1;
    }
    return 0;
}

static int store_index(DIDStore *store, int index)
{
    char string[32];
    int len;

    assert(store);
    assert(index >= 0);

    len = snprintf(string, sizeof(string), "%d", index);
    if (len < 0 || len > sizeof(string)) {
        DIDError_Set(DIDERR_UNKNOWN, "Unknown error.");
        return -1;
    }
    return store_index_string(store, string);
}

static int list_did_helper(const char *path, void *context)
{
    DID_List_Helper *dh = (DID_List_Helper*)context;
    char didpath[PATH_MAX];
    DID did;
    int rc = 0, len;

    if (!path)
        return dh->cb(NULL, dh->context);

    if (strcmp(path, ".") == 0 || strcmp(path, "..") == 0)
        return 0;

    len = snprintf(didpath, sizeof(didpath), "%s/%s/%s", dh->store->root, DID_DIR, path);
    if (len < 0 || len > sizeof(didpath))
        return -1;

    if (test_path(didpath) == S_IFREG || strlen(path) >= sizeof(did.idstring)) {
        delete_file(didpath);
        return 0;
    }

    strcpy(did.idstring, path);
    load_didmeta(dh->store, &did.metadata, did.idstring);

    if (dh->filter == 0 || (dh->filter == 1 && DIDSotre_ContainsPrivateKeys(dh->store, &did)) ||
            (dh->filter == 2 && !DIDSotre_ContainsPrivateKeys(dh->store, &did)))
        rc = dh->cb(&did, dh->context);

    DIDMetaData_Free(&did.metadata);
    return rc;
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
    free((void*)data);
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
    free((void*)data);
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
    load_credmeta(ch->store, &id.metadata, id.did.idstring, id.fragment);
    rc = ch->cb(&id, ch->context);
    CredentialMetaData_Free(&id.metadata);
    return rc;
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
    DIDDocumentBuilder_Destroy(builder);
    if (!document)
        return NULL;

    DIDMetaData_SetAlias(&document->metadata, alias);
    DIDMetaData_SetDeactivated(&document->metadata, false);
    memcpy(&document->did.metadata, &document->metadata, sizeof(DIDMetaData));
    return document;
}

static int store_credential(DIDStore *store, Credential *credential)
{
    const char *data;
    char path[PATH_MAX];
    DIDURL *id;
    time_t lastmodified;
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
        free((void*)data);
        return -1;
    }

    rc = store_file(path, data);
    free((void*)data);
    if (!rc) {
        //set credential last modified
        lastmodified = CredentialMetaData_GetLastModified(&credential->metadata);
        if (lastmodified > 0) {
            set_file_lastmodified(path, lastmodified);
        } else {
            CredentialMetaData_SetLastModified(&credential->metadata, get_file_lastmodified(path));
        }
        return 0;
    }

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

    if (!root || !*root) {
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

    if (mkdirs(root, S_IRWXU) == 0 && !create_store(didstore))
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

int DIDStore_StoreDID(DIDStore *store, DIDDocument *document)
{
    char path[PATH_MAX];
    const char *data, *root;
    char txid[ELA_MAX_TXID_LEN];
    DIDMetaData meta;
    time_t lastmodified;
    ssize_t count;
    int rc;

    if (!store || !document) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    if (load_didmeta(store, &meta, document->did.idstring) == -1)
        return -1;

    rc = DIDMetaData_Merge(&document->metadata, &meta);
    DIDMetaData_Free(&meta);
    if (rc < 0)
        return -1;

    DIDMetaData_SetStore(&document->metadata, store);
	data = DIDDocument_ToJson(document, true);
	if (!data)
		return -1;

    rc = get_file(path, 1, 4, store->root, DID_DIR, document->did.idstring, DOCUMENT_FILE);
    if (rc < 0) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Create file for did document failed.");
        free((void*)data);
        return -1;
    }

    rc = store_file(path, data);
    free((void*)data);
    if (rc) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Store did document failed.");
        goto errorExit;
    }

    if (store_didmeta(store, &document->metadata, &document->did) == -1)
        goto errorExit;

    //set document last modified
    lastmodified = DIDMetaData_GetLastModified(&document->metadata);
    if (lastmodified > 0) {
        set_file_lastmodified(path, lastmodified);
    } else {
        DIDMetaData_SetLastModified(&document->metadata, get_file_lastmodified(path));
    }

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
    free((void*)data);
    if (!document)
        return NULL;

    if (load_didmeta(store, &document->metadata, document->did.idstring) == -1) {
        DIDDocument_Destroy(document);
        return NULL;
    }

    DIDMetaData_SetStore(&document->metadata, store);
    memcpy(&document->did.metadata, &document->metadata, sizeof(DIDMetaData));

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

int DIDStore_StoreCredential(DIDStore *store, Credential *credential)
{
    CredentialMetaData meta;
    char _alias[ELA_MAX_ALIAS_LEN];
    DIDURL *id;

    if (!store || !credential) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    id = Credential_GetId(credential);
    if (!id)
        return -1;

    if (load_credmeta(store, &meta, id->did.idstring, id->fragment) == -1)
        return -1;

    if (CredentialMetaData_Merge(&credential->metadata, &meta) < 0)
        return -1;

    CredentialMetaData_SetStore(&credential->metadata, store);
    CredentialMetaData_Free(&meta);

    if (store_credential(store, credential) == -1 ||
            store_credmeta(store, &credential->metadata, id) == -1)
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
    free((void*)data);
    if (!credential)
        return NULL;

    if (DIDStore_LoadCredMeta(store, &credential->metadata, id) == -1) {
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

int DIDStore_StorePrivateKey_Internal(DIDStore *store, DID *did, DIDURL *id,
        const char *prvkey)
{
    char path[PATH_MAX];

    if (!store || !did || !id || !prvkey || !*prvkey) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    if (!DID_Equals(DIDURL_GetDid(id), did)) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Private key does not match with did.");
        return -1;
    }

    if (get_file(path, 1, 5, store->root, DID_DIR, did->idstring,
            PRIVATEKEYS_DIR, id->fragment) == -1) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Create private key file failed.");
        return -1;
    }

    if (!store_file(path, prvkey))
        return 0;

    DIDError_Set(DIDERR_DIDSTORE_ERROR, "Store private key error: %s", path);
    delete_file(path);
    return -1;
}

int DIDStore_StorePrivateKey(DIDStore *store, const char *storepass, DID *did,
        DIDURL *id, const uint8_t *privatekey, size_t size)
{
    char base64[MAX_PRIVATEKEY_BASE64];

    if (!store || !storepass || !*storepass || !did || !id || !privatekey || size <= 0) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    if (!DID_Equals(DIDURL_GetDid(id), did)) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Private key does not match with did.");
        return -1;
    }

    if (encrypt_to_base64(base64, storepass, privatekey, size) == -1) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Encrypt private key failed.");
        return -1;
    }

    return DIDStore_StorePrivateKey_Internal(store, did, id, base64);
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

static int store_pubidentity_from_prvidentity(DIDStore *store, HDKey *hdkey)
{
    ssize_t size;
    HDKey _derivedkey, *predeviedkey;
    uint8_t extendedkey[EXTENDEDKEY_BYTES];

    assert(store);
    assert(hdkey);

    //// Pre-derive publickey path: m/44'/0'/0'
    predeviedkey = HDKey_GetDerivedKey(hdkey, &_derivedkey, 3, 44 | HARDENED,
            0 | HARDENED, 0 | HARDENED);
    if (!predeviedkey)
        return -1;

    size = HDKey_SerializePub(predeviedkey, extendedkey, sizeof(extendedkey));
    if (size == -1) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Serialize extended public key failed.");
        return -1;
    }

    if (store_extendedpubkey(store, extendedkey, size) == -1)
        return -1;

    return 0;
}


static bool DIDStore_ContainsPublicIdentity(DIDStore *store)
{
    const char *seed;
    char path[PATH_MAX];
    struct stat st;

    if (!store) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    if (get_file(path, 0, 3, store->root, PRIVATE_DIR, HDPUBKEY_FILE) == -1) {
        DIDError_Set(DIDERR_NOT_EXISTS, "Public identity don't already exist.");
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

        // For backward compatible, create pre-derived public key if not exist.
        // TODO: Should be remove in the future
        if (!DIDStore_ContainsPublicIdentity(store))
            rc = store_pubidentity_from_prvidentity(store, identity);
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
        const char *idstring, uint8_t *privatekey, size_t size)
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

    if (DIDStore_StorePrivateKey(store, storepass, &did, &id, privatekey, size) == -1)
        return -1;

    return 0;
}

static DIDDocument* default_didstore_merge(DIDDocument *chaincopy, DIDDocument *localcopy)
{
    assert(chaincopy);
    assert(localcopy);

    DIDMetaData_SetPublished(&localcopy->metadata, DIDMetaData_GetPublished(&chaincopy->metadata));
    DIDMetaData_SetSignature(&localcopy->metadata, DIDMetaData_GetSignature(&chaincopy->metadata));
    memcpy(&localcopy->did.metadata, &localcopy->metadata, sizeof(DIDMetaData));

    return localcopy;
}

int DIDStore_Synchronize(DIDStore *store, const char *storepass, DIDStore_MergeCallback *callback)
{
    int rc, nextindex, i = 0, blanks = 0;
    DIDDocument *chainCopy = NULL, *localCopy = NULL, *finalCopy;
    HDKey _identity, *privateIdentity;
    HDKey _derivedkey, *derivedkey;
    uint8_t extendedkey[EXTENDEDKEY_BYTES];
    DID did;

     if (!store || !storepass || !*storepass)
        return -1;

    if (!callback)
        callback = default_didstore_merge;

    privateIdentity = load_privateIdentity(store, storepass, &_identity);
    if (!privateIdentity)
        return -1;

    nextindex = load_index(store);
    if (nextindex < 0) {
        HDKey_Wipe(privateIdentity);
        return -1;
    }

    while (i < nextindex || blanks < 20) {
        derivedkey = HDKey_GetDerivedKey(privateIdentity, &_derivedkey, 5, 44 | HARDENED,
               0 | HARDENED, 0 | HARDENED, 0, i++);
        if (!derivedkey)
            continue;

        if (Init_DID(&did, HDKey_GetAddress(derivedkey)) == 0) {
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
                    const char *local_signature = DIDMetaData_GetSignature(&localCopy->metadata);
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

                if (DIDStore_StoreDID(store, finalCopy) == 0) {
                    if (HDKey_SerializePrv(derivedkey, extendedkey, sizeof(extendedkey)) > 0) {
                        if (store_default_privatekey(store, storepass, HDKey_GetAddress(derivedkey),
                                extendedkey, sizeof(extendedkey)) == 0) {
                            store_index(store, i);
                        }
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
        HDKey_Wipe(derivedkey);
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

static int didstore_initidentity(DIDStore *store, const char *storepass, HDKey *hdkey)
{
    ssize_t size;
    uint8_t extendedkey[EXTENDEDKEY_BYTES];

    assert(store);
    assert(storepass && *storepass);
    assert(hdkey);

    size = HDKey_SerializePrv(hdkey, extendedkey, sizeof(extendedkey));
    if (size == -1) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Serialize extended private key failed.");
        return -1;
    }

    if (store_extendedprvkey(store, extendedkey, size, storepass) == -1)
        return -1;

    memset(extendedkey, 0, sizeof(extendedkey));
    return store_pubidentity_from_prvidentity(store, hdkey);
}

int DIDStore_InitPrivateIdentity(DIDStore *store, const char *storepass,
        const char *mnemonic, const char *passphrase, const char *language, bool force)
{
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

    if (didstore_initidentity(store, storepass, privateIdentity) == -1)
        return -1;

    if (store_mnemonic(store, storepass, (uint8_t *)mnemonic, strlen(mnemonic)) == -1)
        return -1;

    if (store_index(store, 0) == -1)
        return -1;

    return 0;
}

int DIDStore_InitPrivateIdentityFromRootKey(DIDStore *store, const char *storepass,
        const char *extendedprvkey, bool force)
{
    uint8_t extendedkey[EXTENDEDKEY_BYTES];
    char path[PATH_MAX];
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

    privateIdentity = HDKey_FromExtendedKeyBase58(extendedprvkey, strlen(extendedprvkey) + 1, &_privateIdentity);
    if (!privateIdentity) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Initial private identity failed.");
        return -1;
    }

    if (didstore_initidentity(store, storepass, privateIdentity) == -1)
        return -1;

    if (store_index(store, 0) == -1)
        return -1;

    return 0;
}

static HDKey *get_childkey(DIDStore *store, const char *storepass, int index,
        HDKey *derivedkey)
{
    HDKey _identity, *identity, *dkey;

    assert(store);
    assert(index >= 0);
    assert(derivedkey);

    if (!storepass)
        identity = load_publicIdentity(store, &_identity);
    else
        identity = load_privateIdentity(store, storepass, &_identity);

    if (!identity)
        return NULL;

    dkey = HDKey_GetDerivedKey(identity, derivedkey, 5, 44 | HARDENED, 0 | HARDENED,
            0 | HARDENED, 0, index);
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
    HDKey _derivedkey, *derivedkey;
    DID *did;
    int rc;

    if (!store || index < 0) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    derivedkey = get_childkey(store, NULL, index, &_derivedkey);
    if (!derivedkey)
        return NULL;

    did = DID_New(HDKey_GetAddress(derivedkey));
    HDKey_Wipe(derivedkey);
    return did;
}

DIDDocument *DIDStore_NewDIDByIndex(DIDStore *store, const char *storepass,
        int index, const char *alias)
{
    char publickeybase58[MAX_PUBLICKEY_BASE58];
    uint8_t extendedkey[EXTENDEDKEY_BYTES];
    HDKey _derivedkey, *derivedkey;
    DIDDocument *document;
    DID did;

    if (!store || !storepass || !*storepass || index < 0) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return NULL;
    }

    derivedkey = get_childkey(store, storepass, index, &_derivedkey);
    if (!derivedkey)
        return NULL;

    if (Init_DID(&did, HDKey_GetAddress(derivedkey)) == -1) {
        HDKey_Wipe(derivedkey);
        return NULL;
    }

    //check did is exist or not
    document = DIDStore_LoadDID(store, &did);
    if (document) {
        HDKey_Wipe(derivedkey);
        DIDDocument_Destroy(document);
        return NULL;
    }

    if (HDKey_SerializePrv(derivedkey, extendedkey, sizeof(extendedkey)) < 0) {
        HDKey_Wipe(derivedkey);
        return NULL;
    }
    if (store_default_privatekey(store, storepass, HDKey_GetAddress(derivedkey),
            extendedkey, sizeof(extendedkey)) == -1) {
        HDKey_Wipe(derivedkey);
        return NULL;
    }

    document = create_document(store, &did,
            HDKey_GetPublicKeyBase58(derivedkey, publickeybase58, sizeof(publickeybase58)),
            storepass, alias);
    HDKey_Wipe(derivedkey);
    if (!document) {
        DIDStore_DeleteDID(store, &did);
        return NULL;
    }

    if (DIDStore_StoreDID(store, document) == -1) {
        DIDStore_DeleteDID(store, &did);
        DIDDocument_Destroy(document);
        return NULL;
    }

    DIDDocument_SetStore(document, store);
    return document;
}

ssize_t DIDStore_LoadPrivateKey(DIDStore *store, const char *storepass,
        DID *did, DIDURL *key, uint8_t *privatekey, size_t size)
{
    uint8_t extendedkey[EXTENDEDKEY_BYTES];
    HDKey _identity, *identity;

    assert(store);
    assert(did);
    assert(key);
    assert(privatekey);
    assert(size >= PRIVATEKEY_BYTES);

    if (DIDStore_LoadPrivateKey_Internal(store, storepass, did, key, extendedkey, sizeof(extendedkey)) < 0)
        return -1;

    identity = HDKey_FromExtendedKey(extendedkey, sizeof(extendedkey), &_identity);
    memset(extendedkey, 0, sizeof(extendedkey));
    if (!identity)
        return -1;

    memcpy(privatekey, HDKey_GetPrivateKey(identity), PRIVATEKEY_BYTES);
    return PRIVATEKEY_BYTES;
}

int DIDStore_LoadPrivateKey_Internal(DIDStore *store, const char *storepass, DID *did,
        DIDURL *key, uint8_t *extendedkey, size_t size)
{
    ssize_t loadsize, len;
    const char *privatekey_str;
    char path[PATH_MAX];
    HDKey _identity, *identity;
    HDKey _derivedkey, *derivedkey;
    uint8_t loadextendedkey[EXTENDEDKEY_BYTES];
    bool bsuccessed = false;
    ssize_t rc = -1;

    assert(store);
    assert(storepass && *storepass);
    assert(did);
    assert(key);
    assert(extendedkey);
    assert(size >= EXTENDEDKEY_BYTES);

    if (get_file(path, 0, 5, store->root, DID_DIR, did->idstring, PRIVATEKEYS_DIR, key->fragment) == -1) {
        DIDError_Set(DIDERR_NOT_EXISTS, "No private key file.");
        return -1;
    }

    privatekey_str = load_file(path);
    if (!privatekey_str) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "No valid private key.");
        return -1;
    }

    loadsize = decrypt_from_base64(loadextendedkey, storepass, privatekey_str);
    free((void*)privatekey_str);
    if (loadsize == -1) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Decrypt private key failed.");
        return -1;
    }

    if (loadsize == PRIVATEKEY_BYTES) {
        identity = load_privateIdentity(store, storepass, &_identity);
        if (identity) {
            int i;
            for (i = 0; i < 100; i++) {
                memset(&_derivedkey, 0, sizeof(HDKey));
                memset(extendedkey, 0, size);
                derivedkey = HDKey_GetDerivedKey(identity, &_derivedkey, 5, 44 | HARDENED,
                        0 | HARDENED, 0 | HARDENED, 0, i);
                if (!derivedkey)
                    continue;

                if (memcmp(loadextendedkey, HDKey_GetPrivateKey(derivedkey), PRIVATEKEY_BYTES) == 0) {
                    len = HDKey_SerializePrv(derivedkey, extendedkey, size);
                    if (len < 0) {
                        DIDError_Set(DIDERR_CRYPTO_ERROR, "Serialize derived key failed.");
                        goto errorExit;
                    }
                    bsuccessed = true;
                    break;
                }
            }
        }

        if (!bsuccessed) {
            rc = HDKey_PaddingToExtendedPrivateKey(loadextendedkey, PRIVATEKEY_BYTES, extendedkey, size);
            if (rc < 0)
                goto errorExit;
        }

        if (DIDStore_StorePrivateKey(store, storepass, did, key, extendedkey, size) < 0) {
            memset(extendedkey, 0, size);
            goto errorExit;
        }
        rc = 0;
    } else if (loadsize == EXTENDEDKEY_BYTES) {
        memcpy(extendedkey, loadextendedkey, loadsize);
        rc = 0;
    } else {
        goto errorExit;
    }

errorExit:
    memset(loadextendedkey, 0, sizeof(loadextendedkey));
    return rc;
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

    if (DIDStore_LoadPrivateKey(store, storepass, did, key, binkey, sizeof(binkey)) == -1)
        return -1;

    if (ecdsa_sign_base64(sig, binkey, digest, size) == -1) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "ECDSA sign failed.");
        return -1;
    }

    memset(binkey, 0, sizeof(binkey));
    return 0;
}

bool DIDStore_PublishDID(DIDStore *store, const char *storepass, DID *did,
        DIDURL *signkey, bool force)
{
    const char *last_txid, *local_signature, *local_prevsignature, *resolve_signature = NULL;
    DIDDocument *doc = NULL, *resolve_doc = NULL;
    int rc = -1;
    bool successed;

    if (!store || !storepass || !*storepass || !did) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    doc = DIDStore_LoadDID(store, did);
    if (!doc)
        return false;

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
        successed = DIDBackend_Create(&store->backend, doc, signkey, storepass);
    } else {
        if (DIDDocument_IsDeactivated(resolve_doc)) {
            DIDError_Set(DIDERR_EXPIRED, "Did already deactivated.");
            goto errorExit;
        }

        resolve_signature = resolve_doc->proof.signatureValue;
        if (!resolve_signature || !*resolve_signature) {
            DIDError_Set(DIDERR_RESOLVE_ERROR, "Missing resolve signature.");
            goto errorExit;
        }
        last_txid = DIDMetaData_GetTxid(&resolve_doc->metadata);

        if (!force) {
            resolve_signature = DIDDocument_GetProofSignature(resolve_doc);
            if (!resolve_signature || !*resolve_signature) {
                DIDError_Set(DIDERR_DIDSTORE_ERROR, "Missing document signature on chain.");
                goto errorExit;
            }

            local_signature = DIDMetaData_GetSignature(&doc->metadata);
            local_prevsignature = DIDMetaData_GetPrevSignature(&doc->metadata);
            if ((!local_signature || !*local_signature) && (!local_prevsignature || !*local_prevsignature)) {
                DIDError_Set(DIDERR_DIDSTORE_ERROR,
                        "Missing signatures information, DID SDK dosen't know how to handle it, use force mode to ignore checks.");
                goto errorExit;
            } else if (!local_signature || !local_prevsignature) {
                const char *sig = local_signature != NULL ? local_signature : local_prevsignature;
                if (strcmp(sig, resolve_signature)) {
                    DIDError_Set(DIDERR_DIDSTORE_ERROR,
                            "Current copy not based on the lastest on-chain copy.");
                    goto errorExit;
                }
            } else {
                if (strcmp(local_signature, resolve_signature) &&
                        strcmp(local_prevsignature, resolve_signature)) {
                    DIDError_Set(DIDERR_DIDSTORE_ERROR,
                            "Current copy not based on the lastest on-chain copy.");
                    goto errorExit;
                }

            }
        }

        DIDMetaData_SetTxid(&doc->metadata, last_txid);
        DIDMetaData_SetTxid(&doc->did.metadata, last_txid);
        successed = DIDBackend_Update(&store->backend, doc, signkey, storepass);
    }

    if (!successed)
        goto errorExit;

    ResolveCache_Invalid(did);
    //Meta stores the resolved txid and local signature.
    DIDMetaData_SetSignature(&doc->metadata, DIDDocument_GetProofSignature(doc));
    if (resolve_signature)
        DIDMetaData_SetPrevSignature(&doc->metadata, resolve_signature);
    rc = store_didmeta(store, &doc->metadata, &doc->did);

errorExit:
    if (resolve_doc)
        DIDDocument_Destroy(resolve_doc);
    if (doc)
        DIDDocument_Destroy(doc);
    return rc == -1 ? false : true;
}

bool DIDStore_DeactivateDID(DIDStore *store, const char *storepass,
        DID *did, DIDURL *signkey)
{
    const char *txid;
    DIDDocument *doc;
    bool localcopy = false;
    int rc = 0;
    bool successed;

    if (!store || !storepass || !*storepass || !did) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return false;
    }

    doc = DID_Resolve(did, true);
    if (!doc) {
        doc = DIDStore_LoadDID(store, did);
        if (!doc) {
            DIDError_Set(DIDERR_NOT_EXISTS, "No this did.");
            return false;
        } else {
            localcopy = true;
        }
    }
    else {
        DIDMetaData_SetStore(&doc->metadata, store);
        DIDMetaData_SetStore(&doc->did.metadata, store);
    }

    if (!signkey) {
        signkey = DIDDocument_GetDefaultPublicKey(doc);
        if (!signkey) {
            DIDDocument_Destroy(doc);
            DIDError_Set(DIDERR_INVALID_KEY, "Not default key.");
            return false;
        }
    } else {
        if (!DIDDocument_IsAuthenticationKey(doc, signkey)) {
            DIDDocument_Destroy(doc);
            DIDError_Set(DIDERR_INVALID_KEY, "Invalid authentication key.");
            return false;
        }
    }

    successed = DIDBackend_Deactivate(&store->backend, &doc->did, signkey, storepass);
    DIDDocument_Destroy(doc);
    if (successed)
        ResolveCache_Invalid(did);

    return successed;
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
        free((void*)string);
        if (rc < 0)
            DIDError_Set(DIDERR_IO_ERROR, "Store %s failed.", dst);
        return rc;
    }

    //src is encrypted file.
    size = decrypt_from_base64(plain, old, string);
    free((void*)string);
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

static int change_password(DIDStore *store, const char *new, const char *old)
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
    if (!store || !old || !*old || !new || !*new) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    if (change_password(store, new, old) == -1)
        return -1;

    return postchange_password(store);
}

//--------export and import store
static int write_credentials(DIDURL *id, void *context)
{
    Cred_Export_Helper *ch = (Cred_Export_Helper*)context;

    Credential *cred = NULL;
    const char *vc_string = NULL, *meta_string = NULL;
    DID *creddid;
    int rc = -1;

    if (!id) {
        return 0;
    }

    creddid = DIDURL_GetDid(id);
    if (!creddid)
        return -1;

    cred = DIDStore_LoadCredential(ch->store, creddid, id);
    if (!cred)
        return -1;

    CHECK_TO_MSG_ERROREXIT(JsonGenerator_WriteStartObject(ch->gen),
            DIDERR_OUT_OF_MEMORY, "Start 'credential' object failed.");
    CHECK_TO_MSG_ERROREXIT(JsonGenerator_WriteFieldName(ch->gen, "content"),
            DIDERR_OUT_OF_MEMORY, "Write 'vc' failed.");
    CHECK_TO_MSG_ERROREXIT(Credential_ToJson_Internal(ch->gen, cred, creddid, true, false),
            DIDERR_OUT_OF_MEMORY, "Write credential failed.");
    CHECK_TO_MSG_ERROREXIT(JsonGenerator_WriteFieldName(ch->gen, "meta"),
            DIDERR_OUT_OF_MEMORY, "Write 'meta' failed.");
    CHECK_TO_MSG_ERROREXIT(CredentialMetaData_ToJson_Internal(&cred->metadata, ch->gen),
            DIDERR_OUT_OF_MEMORY, "Write credential meta failed.");
    CHECK_TO_MSG_ERROREXIT(JsonGenerator_WriteEndObject(ch->gen),
            DIDERR_OUT_OF_MEMORY, "End 'credential' object failed.");
    vc_string = Credential_ToJson(cred, true);
    if (!vc_string)
        goto errorExit;

    meta_string = CredentialMetaData_ToJson(&cred->metadata);
    if (!meta_string)
        goto errorExit;

    rc = sha256_digest_update(ch->digest, 2, vc_string, strlen(vc_string),
            meta_string, strlen(meta_string));

errorExit:
    if (cred)
        Credential_Destroy(cred);
    if (vc_string)
        free((void*)vc_string);
    if (meta_string)
        free((void*)meta_string);

    return rc;
}

static int export_type(JsonGenerator *gen, Sha256_Digest *digest)
{
    assert(gen);
    assert(digest);

    CHECK_TO_MSG(JsonGenerator_WriteStringField(gen, "type", DID_EXPORT),
            DIDERR_OUT_OF_MEMORY, "Write 'type' failed.");
    CHECK_TO_MSG(sha256_digest_update(digest, 1, DID_EXPORT, strlen(DID_EXPORT)),
            DIDERR_CRYPTO_ERROR, "Sha256 'type' failed.");

    return 0;
}

static int export_id(JsonGenerator *gen, DID *did, Sha256_Digest *digest)
{
    char idstring[ELA_MAX_DID_LEN];
    const char *value;

    assert(gen);
    assert(did);
    assert(digest);

    value = DID_ToString(did, idstring, sizeof(idstring));
    if (!value)
        return -1;

    CHECK_TO_MSG(JsonGenerator_WriteStringField(gen, "id", value),
            DIDERR_OUT_OF_MEMORY, "Write 'id' failed.");
    CHECK_TO_MSG(sha256_digest_update(digest, 1, value, strlen(value)),
            DIDERR_CRYPTO_ERROR, "Sha256 'id' failed.");

    return 0;
}

static int export_created(JsonGenerator *gen, Sha256_Digest *digest)
{
    char timestring[DOC_BUFFER_LEN];
    const char *value;
    time_t created = 0;

    assert(gen);
    assert(digest);

    value = get_time_string(timestring, sizeof(timestring), &created);
    if(!value) {
        DIDError_Set(DIDERR_UNKNOWN, "Get current time failed.");
        return -1;
    }

    CHECK_TO_MSG(JsonGenerator_WriteStringField(gen, "created", value),
            DIDERR_OUT_OF_MEMORY, "Write 'created' failed.");
    CHECK_TO_MSG(sha256_digest_update(digest, 1, value, strlen(value)),
            DIDERR_CRYPTO_ERROR, "Sha256 'created' failed.");

    return 0;
}

static int export_document(JsonGenerator *gen, DIDDocument *doc, Sha256_Digest *digest)
{
    const char *docstring, *meta;
    int rc;

    assert(gen);
    assert(doc);

    CHECK(JsonGenerator_WriteFieldName(gen, "document"));
    CHECK(JsonGenerator_WriteStartObject(gen));
    CHECK(JsonGenerator_WriteFieldName(gen, "content"));
    CHECK(DIDDocument_ToJson_Internal(gen, doc, true, false));
    CHECK(JsonGenerator_WriteFieldName(gen, "meta"));
    CHECK(DIDMetaData_ToJson_Internal(&doc->metadata, gen));
    CHECK(JsonGenerator_WriteEndObject(gen));

    docstring = DIDDocument_ToJson(doc, true);
    if (!docstring)
        return -1;

    meta = DIDMetaData_ToJson(&doc->metadata);
    if (!meta) {
        free((void*)docstring);
        return -1;
    }

    rc = sha256_digest_update(digest, 2, docstring, strlen(docstring), meta, strlen(meta));
    free((void*)docstring);
    free((void*)meta);

    return rc;
}

static int export_creds(JsonGenerator *gen, DIDStore *store, DID *did, Sha256_Digest *digest)
{
    Cred_Export_Helper ch;

    assert(gen);
    assert(store);
    assert(did);

    if (DIDStore_ContainsCredentials(store, did)) {
        CHECK_TO_MSG(JsonGenerator_WriteFieldName(gen, "credential"),
                DIDERR_OUT_OF_MEMORY, "Write 'document' failed.");
        CHECK_TO_MSG(JsonGenerator_WriteStartArray(gen),
                DIDERR_OUT_OF_MEMORY, "Start credential array failed.");

        ch.store = store;
        ch.gen = gen;
        ch.digest = digest;
        CHECK(DIDStore_ListCredentials(store, did, write_credentials, (void*)&ch));
        CHECK_TO_MSG(JsonGenerator_WriteEndArray(gen),
                DIDERR_OUT_OF_MEMORY, "End credential array failed.");
    }

    return 0;
}

static int export_privatekey(JsonGenerator *gen, DIDStore *store, const char *storepass,
        const char *password, DIDDocument *doc, Sha256_Digest *digest)
{
    ssize_t size;
    int rc, i;
    DID *did;
    DIDURL *keyid;
    char _idstring[ELA_MAX_DIDURL_LEN], *idstring;

    assert(gen);
    assert(store);
    assert(digest);

    did = &doc->did;
    if (DIDSotre_ContainsPrivateKeys(store, did)) {
        size = DIDDocument_GetPublicKeyCount(doc);
        if (size == -1)
            return -1;

        PublicKey **pks = (PublicKey**)alloca(size * sizeof(PublicKey*));
        if (!pks)
            return -1;

        rc = DIDDocument_GetPublicKeys(doc, pks, size);
        if (rc < 0)
            return -1;

        CHECK_TO_MSG(JsonGenerator_WriteFieldName(gen, "privatekey"),
                DIDERR_OUT_OF_MEMORY, "Write 'privatekey' failed.");
        CHECK_TO_MSG(JsonGenerator_WriteStartArray(gen),
                DIDERR_OUT_OF_MEMORY, "Start 'privatekey' array failed.");

        for (i = 0; i < size; i++) {
            char base64[512];
            uint8_t extendedkey[EXTENDEDKEY_BYTES];
            keyid = &pks[i]->id;
            if (DIDStore_ContainsPrivateKey(store, did, keyid)) {
                if (DIDStore_LoadPrivateKey_Internal(store, storepass, did, keyid, extendedkey, sizeof(extendedkey)) == -1)
                    return -1;

                rc = encrypt_to_base64(base64, password, extendedkey, sizeof(extendedkey));
                memset(extendedkey, 0, sizeof(extendedkey));
                if (rc < 0)
                    return -1;

                CHECK_TO_MSG(JsonGenerator_WriteStartObject(gen),
                        DIDERR_OUT_OF_MEMORY, "Start 'privatekey' failed.");
                idstring = DIDURL_ToString(keyid, _idstring, sizeof(_idstring), false);
                CHECK_TO_MSG(JsonGenerator_WriteStringField(gen, "id", idstring),
                        DIDERR_OUT_OF_MEMORY, "Write 'id' failed.");
                CHECK_TO_MSG(JsonGenerator_WriteStringField(gen, "key", (char*)base64),
                        DIDERR_OUT_OF_MEMORY, "Write 'key' failed.");
                CHECK_TO_MSG(JsonGenerator_WriteEndObject(gen),
                        DIDERR_OUT_OF_MEMORY, "End 'privatekey' failed.");

                CHECK_TO_MSG(sha256_digest_update(digest, 2, idstring, strlen(idstring), base64, strlen(base64)),
                        DIDERR_CRYPTO_ERROR, "Update digest with privatekey failed.");
            }
        }

        CHECK_TO_MSG(JsonGenerator_WriteEndArray(gen),
                DIDERR_OUT_OF_MEMORY, "End 'privatekey' array failed.");
    }

    return 0;
}

static int export_init(JsonGenerator *gen, const char *password, Sha256_Digest *digest)
{
    assert(gen);
    assert(digest);

    CHECK_TO_MSG(sha256_digest_init(digest),
            DIDERR_CRYPTO_ERROR, "Init sha256 digest failed.");
    CHECK_TO_MSG(sha256_digest_update(digest, 1, password, strlen(password)),
            DIDERR_CRYPTO_ERROR, "Sha256 password failed.");
    CHECK_TO_MSG(JsonGenerator_WriteStartObject(gen),
            DIDERR_OUT_OF_MEMORY, "Write object failed.");

    return 0;
}

static int export_final(JsonGenerator *gen, Sha256_Digest *digest)
{
    char base64[512];
    uint8_t final_digest[SHA256_BYTES];
    ssize_t size;

    assert(gen);
    assert(digest);

    size = sha256_digest_final(digest, final_digest);
    if (size < 0) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Final sha256 digest failed.");
        return -1;
    }

    CHECK_TO_MSG(base64_url_encode(base64, final_digest, size),
            DIDERR_CRYPTO_ERROR, "Final sha256 digest failed.");
    CHECK_TO_MSG(JsonGenerator_WriteStringField(gen, "fingerprint", base64),
            DIDERR_OUT_OF_MEMORY, "Write 'fingerprint' failed.");
    CHECK_TO_MSG(JsonGenerator_WriteEndObject(gen),
            DIDERR_OUT_OF_MEMORY, "End export object failed.");

    return 0;
}

static int exportdid_internal(JsonGenerator *gen, DIDStore *store, const char * storepass,
        DID *did, const char *password)
{
    Sha256_Digest digest;
    DIDDocument *doc;
    int rc = -1;

    assert(gen);
    assert(did);

    doc = DIDStore_LoadDID(store, did);
    if (!doc) {
        DIDError_Set(DIDERR_DIDSTORE_ERROR, "Export DID failed, not exist.");
        return rc;
    }

    if (export_init(gen, password, &digest) < 0 ||
            export_type(gen, &digest) < 0 ||
            export_id(gen, did, &digest) < 0 ||
            export_created(gen, &digest) < 0 ||
            export_document(gen, doc, &digest) < 0 ||
            export_creds(gen, store, did, &digest) < 0 ||
            export_privatekey(gen, store, storepass, password, doc, &digest) < 0 ||
            export_final(gen, &digest) < 0)
        goto errorExit;

    rc = 0;

errorExit:
    sha256_digest_cleanup(&digest);

    if (doc)
        DIDDocument_Destroy(doc);

    return rc;
}

static int check_file(const char *file)
{
    char *path;

    assert(file && *file);

    if (test_path(file) > 0)
        delete_file(file);

    path = alloca(strlen(file) + 1);
    strcpy(path, file);

    char *pos = strrchr(path, '/');
    if (!pos) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid file path.");
        return -1;
    }

    *pos = 0;
    if (mkdirs(path, S_IRWXU) == -1) {
        DIDError_Set(DIDERR_IO_ERROR, "Create the directory of file failed.");
        return -1;
    }
    *path = '/';
    return 0;
}

int DIDStore_ExportDID(DIDStore *store, const char *storepass, DID *did,
        const char *file, const char *password)
{
    JsonGenerator g, *gen;
    const char *data;
    int rc;

    if (!store || !storepass || !*storepass || !did || !file || !*file ||
            !password || !*password) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    //check file
    if (check_file(file) < 0)
        return -1;

    //generate did export string
    gen = JsonGenerator_Initialize(&g);
    if (!gen) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Json generator initialize failed.");
        return -1;;
    }

    if (exportdid_internal(gen, store, storepass, did, password) < 0) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Serialize exporting did to json failed.");
        JsonGenerator_Destroy(gen);
        return -1;
    }

    data = JsonGenerator_Finish(gen);
    rc = store_file(file, data);
    free((void*)data);
    if (rc < 0) {
        DIDError_Set(DIDERR_IO_ERROR, "write exporting did string into file failed.");
        return -1;
    }

    return 0;
}

static int import_type(cJSON *json, Sha256_Digest *digest)
{
    cJSON *item;

    assert(json);
    assert(digest);

    item = cJSON_GetObjectItem(json, "type");
    if (!item) {
        DIDError_Set(DIDERR_NOT_EXISTS, "Missing export did type.");
        return -1;
    }
    if (!cJSON_IsString(item)) {
        DIDError_Set(DIDERR_UNSUPPOTED, "Invalid export did type.");
        return -1;
    }

    if (strcmp(item->valuestring, DID_EXPORT)) {
        DIDError_Set(DIDERR_UNSUPPOTED, "Invalid export data, unknown type.");
        return -1;
    }

    CHECK_TO_MSG(sha256_digest_update(digest, 1, item->valuestring, strlen(item->valuestring)),
            DIDERR_CRYPTO_ERROR, "Sha256 'type' failed.");

    return 0;
}

static DID *import_id(cJSON *json, Sha256_Digest *digest)
{
    cJSON *item;

    assert(json);
    assert(digest);

    item = cJSON_GetObjectItem(json, "id");
    if (!item) {
        DIDError_Set(DIDERR_NOT_EXISTS, "Missing export did.");
        return NULL;
    }
    if (!cJSON_IsString(item)) {
        DIDError_Set(DIDERR_UNSUPPOTED, "Invalid export did.");
        return NULL;
    }

    if (sha256_digest_update(digest, 1, item->valuestring, strlen(item->valuestring)) < 0) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Sha256 'id' failed.");
        return NULL;
    }

    return DID_FromString(item->valuestring);
}

static int import_created(cJSON *json, Sha256_Digest *digest)
{
    cJSON *item;

    assert(json);
    assert(digest);

    item = cJSON_GetObjectItem(json, "created");
    if (!item) {
        DIDError_Set(DIDERR_NOT_EXISTS, "Missing created time.");
        return -1;
    }
    if (!cJSON_IsString(item)) {
        DIDError_Set(DIDERR_UNSUPPOTED, "Invalid created time.");
        return -1;
    }

    if (sha256_digest_update(digest, 1, item->valuestring, strlen(item->valuestring)) < 0) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Sha256 'created' failed.");
        return -1;
    }

    return 0;
}

static DIDDocument *import_document(cJSON *json, DID *did, Sha256_Digest *digest)
{
    cJSON *item, *field;
    DIDDocument *doc = NULL;
    const char *docstring = NULL, *metastring = NULL;
    int rc;

    assert(json);
    assert(did);
    assert(digest);

    item = cJSON_GetObjectItem(json, "document");
    if (!item) {
        DIDError_Set(DIDERR_NOT_EXISTS, "Missing created time.");
        return NULL;
    }
    if (!cJSON_IsObject(item)) {
        DIDError_Set(DIDERR_UNSUPPOTED, "Invalid 'document'.");
        return NULL;
    }

    field = cJSON_GetObjectItem(item, "content");
    if (!field) {
        DIDError_Set(DIDERR_NOT_EXISTS, "Missing document 'content'.");
        return NULL;
    }
    if (!cJSON_IsObject(field)) {
        DIDError_Set(DIDERR_UNSUPPOTED, "Invalid document 'content'.");
        return NULL;
    }

    doc = DIDDocument_FromJson_Internal(field);
    if (!doc)
        return NULL;

    if (!DID_Equals(&doc->did, did) || !DIDDocument_IsGenuine(doc)) {
        DIDError_Set(DIDERR_UNSUPPOTED, "Invalid DID document in the export data.");
        goto errorExit;
    }

    docstring = DIDDocument_ToJson(doc, true);
    if (!docstring)
        goto errorExit;

    field = cJSON_GetObjectItem(item, "meta");
    if (!field) {
        DIDError_Set(DIDERR_NOT_EXISTS, "Missing 'meta'.");
        goto errorExit;
    }
    if (!cJSON_IsObject(field)) {
        DIDError_Set(DIDERR_UNSUPPOTED, "Invalid 'meta'.");
        goto errorExit;
    }

    if (DIDMetaData_FromJson_Internal(&doc->metadata, field) < 0)
        goto errorExit;

    memcpy(&doc->did.metadata, &doc->metadata, sizeof(DIDMetaData));
    metastring = DIDMetaData_ToJson(&doc->metadata);
    if (!metastring)
        goto errorExit;

    rc = sha256_digest_update(digest, 2, docstring, strlen(docstring), metastring, strlen(metastring));
    free((void*)docstring);
    free((void*)metastring);
    if (rc < 0) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Sha256 'document' failed.");
        DIDDocument_Destroy(doc);
        return NULL;
    }

    return doc;

errorExit:
    if (doc)
        DIDDocument_Destroy(doc);
    if (docstring)
        free((void*)docstring);
    if (metastring)
        free((void*)metastring);

    return NULL;
}

static int import_creds_count(cJSON *json)
{
    cJSON *item;

    assert(json);

    item = cJSON_GetObjectItem(json, "credential");
    if (!item)
        return 0;

    if (!cJSON_IsArray(item)) {
        DIDError_Set(DIDERR_UNKNOWN, "Unknown credential.");
        return -1;
    }

    return cJSON_GetArraySize(item);
}

static ssize_t import_creds(cJSON *json, DID *did, Credential **creds, size_t size,
        Sha256_Digest *digest)
{
    cJSON *item, *field, *child_field;
    ssize_t count;
    int i, rc;

    assert(json);
    assert(creds);
    assert(size > 0);

    item = cJSON_GetObjectItem(json, "credential");
    if (!item)
        return 0;

    if (!cJSON_IsArray(item)) {
        DIDError_Set(DIDERR_UNKNOWN, "Unknown 'credential'.");
        return -1;
    }

    count = cJSON_GetArraySize(item);
    if (count == 0 || count > size) {
        DIDError_Set(DIDERR_UNSUPPOTED, "Invalid 'credential'.");
        return -1;
    }

    for (i = 0; i < count; i++) {
        field = cJSON_GetArrayItem(item, i);
        child_field = cJSON_GetObjectItem(field, "content");
        if (!child_field) {
            DIDError_Set(DIDERR_NOT_EXISTS, "Missing credential 'content'.");
            goto errorExit;
        }
        if (!cJSON_IsObject(child_field)) {
            DIDError_Set(DIDERR_UNSUPPOTED, "Invalid credential 'content'.");
            goto errorExit;
        }

        Credential *cred = Parse_Credential(child_field, did);
        if (!cred)
            goto errorExit;

        child_field = cJSON_GetObjectItem(field, "meta");
        if (!child_field) {
            DIDError_Set(DIDERR_NOT_EXISTS, "Missing 'meta'.");
            goto errorExit;
        }
        if (!cJSON_IsObject(child_field)) {
            DIDError_Set(DIDERR_UNSUPPOTED, "Invalid 'meta'.");
            goto errorExit;
        }
        if (CredentialMetaData_FromJson_Internal(&cred->metadata, child_field) < 0)
            goto errorExit;

        const char *credstring = Credential_ToJson(cred, true);
        if (!credstring)
            goto errorExit;

        const char *metastring = CredentialMetaData_ToJson(&cred->metadata);
        if (!metastring) {
            free((void*)credstring);
            goto errorExit;
        }

        rc = sha256_digest_update(digest, 2, credstring, strlen(credstring), metastring, strlen(metastring));
        free((void*)credstring);
        free((void*)metastring);
        if (rc < 0)
            goto errorExit;

        creds[i] = cred;
    }

    return i;

errorExit:
    if (i > 0) {
        for (int j = 0; j < i; j++)
            Credential_Destroy(creds[j]);
    }

    return -1;
}

static ssize_t import_privatekey_count(cJSON *json)
{
    cJSON *item;

    assert(json);

    item = cJSON_GetObjectItem(json, "privatekey");
    if (!item) {
        DIDError_Set(DIDERR_NOT_EXISTS, "Missing 'privatekey'.");
        return -1;
    }
    if (!cJSON_IsArray(item)) {
        DIDError_Set(DIDERR_UNSUPPOTED, "Invalid 'privatekey'.");
        return -1;
    }

    return cJSON_GetArraySize(item);
}

static ssize_t import_privatekey(cJSON *json, const char *storepass, const char *password,
       DID *did, Prvkey_Export *prvs, size_t size, Sha256_Digest *digest)
{
    cJSON *item, *field, *id_field, *key_field;
    ssize_t count;
    uint8_t binkey[EXTENDEDKEY_BYTES];
    char privatekey[512];
    size_t keysize;
    int i = 0;

    assert(json);
    assert(storepass && *storepass);
    assert(password && *password);
    assert(did);
    assert(prvs);
    assert(size > 0);
    assert(digest);

    item = cJSON_GetObjectItem(json, "privatekey");
    if (!item) {
        DIDError_Set(DIDERR_NOT_EXISTS, "Missing 'privatekey'.");
        return -1;
    }
    if (!cJSON_IsArray(item)) {
        DIDError_Set(DIDERR_UNSUPPOTED, "Invalid 'privatekey' array.");
        return -1;
    }

    count = cJSON_GetArraySize(item);
    if (count == 0) {
        DIDError_Set(DIDERR_UNSUPPOTED, "Invalid 'privatekey' array.");
        return -1;
    }
    if (count > size) {
        DIDError_Set(DIDERR_UNSUPPOTED, "Please give larger buffer for private keys.");
        return -1;
    }

    for (i = 0; i < count; i++) {
        field = cJSON_GetArrayItem(item, i);
        if (!field) {
            DIDError_Set(DIDERR_NOT_EXISTS, "Missing 'privatekey' item.");
            return -1;
        }
        if (!cJSON_IsObject(field)) {
            DIDError_Set(DIDERR_UNSUPPOTED, "Invalid 'privatekey'.");
            return -1;
        }
        id_field = cJSON_GetObjectItem(field, "id");
        if (!id_field) {
            DIDError_Set(DIDERR_NOT_EXISTS, "Missing 'id' in 'privatekey' failed.");
            return -1;
        }
        if (!cJSON_IsString(id_field)) {
            DIDError_Set(DIDERR_UNSUPPOTED, "Invalid 'id' in 'privatekey' failed.");
            return -1;
        }

        DIDURL *id = DIDURL_FromString(id_field->valuestring, did);
        if (!id)
            return -1;

        DIDURL_Copy(&prvs[i].keyid, id);
        DIDURL_Destroy(id);

        key_field = cJSON_GetObjectItem(field, "key");
        if (!key_field) {
            DIDError_Set(DIDERR_NOT_EXISTS, "Missing 'key' in 'privatekey'.");
            return -1;
        }
        if (!cJSON_IsString(key_field)) {
            DIDError_Set(DIDERR_UNSUPPOTED, "Invalid 'key' in 'privatekey'.");
            return -1;
        }

        keysize = decrypt_from_base64(binkey, password, key_field->valuestring);
        if (keysize < 0) {
            DIDError_Set(DIDERR_CRYPTO_ERROR, "Decrypt private key failed.");
            return -1;
        }

        keysize = encrypt_to_base64(privatekey, storepass, binkey, keysize);
        memset(binkey, 0, sizeof(binkey));
        if (keysize < 0) {
            DIDError_Set(DIDERR_CRYPTO_ERROR, "Encrypt private key failed.");
            return -1;
        }
        memcpy(prvs[i].key, privatekey, keysize);

        if (sha256_digest_update(digest, 2, id_field->valuestring, strlen(id_field->valuestring),
                    key_field->valuestring, strlen(key_field->valuestring)) < 0) {
            DIDError_Set(DIDERR_CRYPTO_ERROR, "Sha256 'key' in 'privatekey' failed.");
            return -1;
        }
    }
    return i;
}

static int import_fingerprint(cJSON *json, Sha256_Digest *digest)
{
    cJSON *item;
    uint8_t final_digest[SHA256_BYTES];
    char base64[512];
    ssize_t size;

    assert(json);
    assert(digest);

    item = cJSON_GetObjectItem(json, "fingerprint");
    if (!item) {
        DIDError_Set(DIDERR_NOT_EXISTS, "Missing 'fingerprint'.");
        return -1;
    }
    if (!cJSON_IsString(item)) {
        DIDError_Set(DIDERR_UNSUPPOTED, "Invalid 'fingerprint'.");
        return -1;
    }

    size = sha256_digest_final(digest, final_digest);
    if (size < 0) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Final sha256 digest failed.");
        return -1;
    }

    if (base64_url_encode(base64, final_digest, size) < 0) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Encrypt digest failed.");
        return -1;
    }
    if (strcmp(base64, item->valuestring)) {
        DIDError_Set(DIDERR_MALFORMED_EXPORTDID, "Invalid export data, the fingerprint mismatch.");
        return -1;
    }

    return 0;
}

static int import_init(const char *password, Sha256_Digest *digest)
{
    CHECK_TO_MSG(sha256_digest_init(digest),
            DIDERR_CRYPTO_ERROR, "Init sha256 digest failed.");
    CHECK_TO_MSG(sha256_digest_update(digest, 1, password, strlen(password)),
            DIDERR_CRYPTO_ERROR, "Sha256 password failed.");
    return 0;
}

int DIDStore_ImportDID(DIDStore *store, const char *storepass,
        const char *file, const char *password)
{
    const char *string = NULL;
    cJSON *root = NULL;
    Sha256_Digest digest;
    DID *did = NULL;
    DIDDocument *doc = NULL;
    Credential **creds = NULL;
    Prvkey_Export *prvs;
    int rc = -1, i;

    if (!store || !storepass || !*storepass || !file || !*file ||
            !password || !*password) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    if (test_path(file) != S_IFREG)
        return -1;

    string = load_file(file);
    if (!string) {
        DIDError_Set(DIDERR_IO_ERROR, "Load export did file failed.");
        return -1;
    }

    root = cJSON_Parse(string);
    free((void*)string);
    if (!root) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Deserialize export file from json failed.");
        return -1;
    }

    if (import_init(password, &digest) < 0)
        goto errorExit;

    //type
    if (import_type(root, &digest) < 0)
        goto errorExit;

    //id
    did = import_id(root, &digest);
    if (!did)
        goto errorExit;

    //created
    if (import_created(root, &digest) < 0)
        goto errorExit;

    //document
    doc = import_document(root, did, &digest);
    if (!doc)
        goto errorExit;

    //credential
    size_t cred_size = import_creds_count(root);
    if (cred_size < 0)
        goto errorExit;
    if (cred_size > 0) {
        creds = (Credential**)alloca(cred_size * sizeof(Credential*));
        cred_size = import_creds(root, did, creds, cred_size, &digest);
        if (cred_size < 0)
            goto errorExit;
    }

    //privatekey
    size_t prv_size = import_privatekey_count(root);
    if (prv_size < 0)
        goto errorExit;
    if (prv_size > 0) {
        prvs = (Prvkey_Export*)alloca(prv_size * sizeof(Prvkey_Export));
        memset(prvs, 0, prv_size * sizeof(Prvkey_Export));
        prv_size = import_privatekey(root, storepass, password, did, prvs, prv_size, &digest);
        if (prv_size < 0)
            goto errorExit;
    }

    //fingerprint
    if (import_fingerprint(root, &digest) < 0)
        goto errorExit;

    //save all files
    if (DIDStore_StoreDID(store, doc) < 0)
        goto errorExit;

    for (i = 0; i < cred_size; i++) {
        if (DIDStore_StoreCredential(store, creds[i]) < 0)
            goto errorExit;
    }

    for (i = 0; i < prv_size; i++) {
        if (DIDStore_StorePrivateKey_Internal(store, did, &prvs[i].keyid, prvs[i].key) < 0)
            goto errorExit;
    }

    rc = 0;

errorExit:
    if (rc == -1)
        sha256_digest_cleanup(&digest);
    if (root)
        cJSON_Delete(root);
    if (did)
        DID_Destroy(did);
    if (doc)
        DIDDocument_Destroy(doc);
    if (creds) {
        for (int i = 0; i < cred_size; i++)
            Credential_Destroy(creds[i]);
    }

    return rc;
}

static int export_prvkey(JsonGenerator *gen, DIDStore *store, const char *storepass,
        const char *password, Sha256_Digest *digest)
{
    uint8_t extendedkey[EXTENDEDKEY_BYTES];
    char encryptedKey[512];
    ssize_t size;

    assert(gen);
    assert(store);
    assert(storepass && *storepass);
    assert(password && *password);
    assert(digest);

    size = load_extendedprvkey(store, extendedkey, sizeof(extendedkey), storepass);
    if (size < 0)
        return -1;

    size = encrypt_to_base64(encryptedKey, password, extendedkey, size);
    memset(extendedkey, 0, sizeof(extendedkey));
    if (size < 0) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Encrypt extended private identity failed.");
        return -1;
    }

    CHECK_TO_MSG(JsonGenerator_WriteStringField(gen, "key", encryptedKey),
        DIDERR_OUT_OF_MEMORY, "Write 'key' failed.");
    CHECK_TO_MSG(sha256_digest_update(digest, 1, encryptedKey, strlen(encryptedKey)),
        DIDERR_CRYPTO_ERROR, "Sha256 'key' failed.");

    return 0;
}

static int export_mnemonic(JsonGenerator *gen, DIDStore *store, const char *storepass,
        const char *password, Sha256_Digest *digest)
{
    char mnemonic[ELA_MAX_MNEMONIC_LEN], encryptedmnemonic[512];
    ssize_t size;

    assert(gen);
    assert(store);
    assert(storepass && *storepass);
    assert(password && *password);
    assert(digest);

    size = load_mnemonic(store, storepass, mnemonic, sizeof(mnemonic));
    if (size < 0)
        return -1;

    size = encrypt_to_base64(encryptedmnemonic, password, (uint8_t*)mnemonic, size - 1);
    memset(mnemonic, 0, sizeof(mnemonic));
    if (size < 0)
        return -1;

    CHECK_TO_MSG(JsonGenerator_WriteStringField(gen, "mnemonic", encryptedmnemonic),
            DIDERR_OUT_OF_MEMORY, "Write 'mnemonic' failed.");
    CHECK_TO_MSG(sha256_digest_update(digest, 1, encryptedmnemonic, strlen(encryptedmnemonic)),
           DIDERR_CRYPTO_ERROR, "Sha256 'mnemonic' failed.");

    return 0;
}

static int export_pubkey(JsonGenerator *gen, DIDStore *store, Sha256_Digest *digest)
{
    const char *pubKey = NULL;
    int rc = -1;

    assert(gen);
    assert(store);
    assert(digest);

    pubKey = load_pubkey_file(store);
    if (!pubKey)
        return 0;

    CHECK_TO_MSG(JsonGenerator_WriteStringField(gen, "key.pub", pubKey),
            DIDERR_OUT_OF_MEMORY, "Write 'key.pub' failed.");
    CHECK_TO_MSG(sha256_digest_update(digest, 1, pubKey, strlen(pubKey)),
            DIDERR_CRYPTO_ERROR, "Sha256 'key.pub' failed.");

    rc = 0;

errorExit:
    if (pubKey)
        free((void*)pubKey);

    return rc;
}

static int export_index(JsonGenerator *gen, DIDStore *store, Sha256_Digest *digest)
{
    char string[32];
    int len, index;

    assert(gen);
    assert(store);
    assert(digest);

    index = load_index(store);
    len = snprintf(string, sizeof(string), "%d", index);
    if (len < 0 || len > sizeof(string))
        return -1;

    CHECK_TO_MSG(JsonGenerator_WriteStringField(gen, "index", string),
            DIDERR_OUT_OF_MEMORY, "Write 'index' failed.");
    CHECK_TO_MSG(sha256_digest_update(digest, 1, string, strlen(string)),
            DIDERR_CRYPTO_ERROR, "Sha256 'index' failed.");

    return 0;
}

int DIDStore_ExportPrivateIdentity(DIDStore *store, const char *storepass,
        const char *file, const char *password)
{
    Sha256_Digest digest;

    const char *pubKey = NULL, *data;
    ssize_t size;
    int index, len, rc = -1;
    JsonGenerator g, *gen;

    if (!store || !storepass || !*storepass || !file || !*file ||
            !password || !*password) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    if (check_file(file) < 0)
        return -1;

    gen = JsonGenerator_Initialize(&g);
    if (!gen) {
        DIDError_Set(DIDERR_OUT_OF_MEMORY, "Json generator initialize failed.");
        goto errorExit;
    }

    if (export_init(gen, password, &digest) < 0 || export_type(gen, &digest) < 0)
        goto errorExit;

    //private extended key
    if (export_mnemonic(gen, store, storepass, password, &digest) < 0 ||
            export_prvkey(gen, store, storepass, password, &digest) < 0 ||
            export_pubkey(gen, store, &digest) < 0 ||
            export_index(gen, store, &digest) < 0)
        return -1;

    if (export_final(gen, &digest) < 0)
        goto errorExit;

    data = JsonGenerator_Finish(gen);
    rc = store_file(file, data);
    free((void*)data);
    if (rc < 0) {
        DIDError_Set(DIDERR_IO_ERROR, "write exporting did string into file failed.");
        goto errorExit;
    }

    rc = 0;

errorExit:
    if (pubKey)
       free((void*)pubKey);

    return rc;
}

int DIDStore_ImportPrivateIdentity(DIDStore *store, const char *storepass,
        const char *file, const char *password)
{
    cJSON *root = NULL, *item;
    const char *string = NULL;
    uint8_t mnemonic[ELA_MAX_MNEMONIC_LEN], extendedkey[EXTENDEDKEY_BYTES];
    uint8_t final_digest[SHA256_BYTES];
    ssize_t size;
    char base64[512];
    Sha256_Digest digest;
    int rc = -1;

    if (!store || !storepass || !*storepass || !file || !*file ||
            !password || !*password) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    if (test_path(file) != S_IFREG)
        return -1;

    string = load_file(file);
    if (!string) {
        DIDError_Set(DIDERR_IO_ERROR, "Load export did file failed.");
        return -1;
    }

    root = cJSON_Parse(string);
    free((void*)string);
    if (!root) {
        DIDError_Set(DIDERR_MALFORMED_DOCUMENT, "Deserialize export file from json failed.");
        return -1;
    }

    if (import_init(password, &digest) < 0 || import_type(root, &digest) < 0)
        return -1;

    //mnemonic
    item = cJSON_GetObjectItem(root, "mnemonic");
    if (!item) {
        DIDError_Set(DIDERR_NOT_EXISTS, "Missing 'mnemonic'.");
        return -1;
    }
    if (!cJSON_IsString(item)) {
        DIDError_Set(DIDERR_UNKNOWN, "Invalid 'mnemonic'.");
        return -1;
    }

    size = decrypt_from_base64(mnemonic, password, item->valuestring);
    if (size < 0) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Decrypt mnemonic failed.");
        return -1;
    }

    CHECK_TO_ERROREXIT(store_mnemonic(store, storepass, mnemonic, size));
    CHECK_TO_MSG_ERROREXIT(sha256_digest_update(&digest, 1, item->valuestring, strlen(item->valuestring)),
            DIDERR_CRYPTO_ERROR, "Sha256 'mnemonic' failed.");

    //prvkey
    item = cJSON_GetObjectItem(root, "key");
    if (!item) {
        DIDError_Set(DIDERR_NOT_EXISTS, "Missing 'key'.");
        return -1;
    }
    if (!cJSON_IsString(item)) {
        DIDError_Set(DIDERR_UNSUPPOTED, "Invalid 'key'.");
        return -1;
    }
    memset(extendedkey, 0, sizeof(extendedkey));
    size = decrypt_from_base64(extendedkey, password, item->valuestring);
    if (size < 0) {
        DIDError_Set(DIDERR_CRYPTO_ERROR, "Decrypt 'key' failed.");
        return -1;
    }

    CHECK_TO_ERROREXIT(store_extendedprvkey(store, extendedkey, size, storepass));
    memset(extendedkey, 0, sizeof(extendedkey));
    CHECK_TO_MSG_ERROREXIT(sha256_digest_update(&digest, 1, item->valuestring, strlen(item->valuestring)),
            DIDERR_CRYPTO_ERROR, "Sha256 'key' failed.");

    //pubkey
    item = cJSON_GetObjectItem(root, "key.pub");
    if (item) {
        if (!cJSON_IsString(item)) {
            DIDError_Set(DIDERR_UNSUPPOTED, "Invalid 'key.pub'.");
            return -1;
        }

        CHECK_TO_ERROREXIT(store_pubkey(store, item->valuestring));
        CHECK_TO_MSG_ERROREXIT(sha256_digest_update(&digest, 1, item->valuestring, strlen(item->valuestring)),
                DIDERR_CRYPTO_ERROR, "Sha256 'key.pub' failed.");
    }

    //index
    item = cJSON_GetObjectItem(root, "index");
    if (!item) {
        DIDError_Set(DIDERR_NOT_EXISTS, "Missing 'index'.");
        return -1;
    }
    if (!cJSON_IsString(item)) {
        DIDError_Set(DIDERR_UNSUPPOTED, "Invalid 'index'.");
        return -1;
    }
    CHECK_TO_ERROREXIT(store_index_string(store, item->valuestring));
    CHECK_TO_MSG_ERROREXIT(sha256_digest_update(&digest, 1, item->valuestring, strlen(item->valuestring)),
            DIDERR_CRYPTO_ERROR, "Sha256 'index' failed.");

    //fingerprint
    item = cJSON_GetObjectItem(root, "fingerprint");
    if (!item) {
        DIDError_Set(DIDERR_NOT_EXISTS, "Missing 'fingerprint'.");
        return -1;
    }
    if (!cJSON_IsString(item)) {
        DIDError_Set(DIDERR_UNSUPPOTED, "Invalid 'fingerprint'.");
        return -1;
    }

    CHECK_TO_MSG_ERROREXIT(sha256_digest_final(&digest, final_digest),
            DIDERR_CRYPTO_ERROR, "Sha256 final failed.");
    CHECK_TO_MSG(base64_url_encode(base64, final_digest, sizeof(final_digest)),
            DIDERR_CRYPTO_ERROR, "Final sha256 digest failed.");
    if (strcmp(base64, item->valuestring)) {
        DIDError_Set(DIDERR_MALFORMED_EXPORTDID, "Invalid export data, the fingerprint mismatch.");
        goto errorExit;
    }

    rc = 0;
errorExit:
    if (root)
       cJSON_Delete(root);

   return rc;
}

static zip_t *create_zip(const char *file)
{
    int err;
    zip_t *zip;

    assert(file && *file);

    if ((zip = zip_open(file, ZIP_CREATE | ZIP_TRUNCATE, &err)) == NULL) {
        zip_error_t error;
        zip_error_init_with_code(&error, err);
        DIDError_Set(DIDERR_MALFORMED_EXPORTDID, "Can't open zip archive '%s': %s", file, zip_error_strerror(&error));
        zip_error_fini(&error);
    }

    return zip;
}

static int did_to_zip(DID *did, void *context)
{
    DID_Export *export = (DID_Export*)context;
    char tmpfile[PATH_MAX];

    if (!did)
        return 0;

    sprintf(tmpfile, "%s/%s.json", export->tmpdir, did->idstring);
    delete_file(tmpfile);
    if (DIDStore_ExportDID(export->store, export->storepass, did, tmpfile, export->password) < 0)
       return -1;

    zip_source_t *did_source = zip_source_file(export->zip, tmpfile, 0, 0);

    if (!did_source) {
        DIDError_Set(DIDERR_MALFORMED_EXPORTDID, "Get source file failed.");
        return -1;
    }

    if (zip_file_add(export->zip, did->idstring, did_source, 0) < 0) {
        zip_source_free(did_source);
        DIDError_Set(DIDERR_MALFORMED_EXPORTDID, "Add source file failed.");
        return -1;
    }

    return 0;
}

static int exportdid_to_zip(DIDStore *store, const char *storepass, zip_t *zip,
        const char *password, const char *tmpdir)
{
    DID_Export export;

    assert(store);
    assert(storepass && *storepass);
    assert(zip);
    assert(password && *password);

    export.store = store;
    export.storepass = storepass;
    export.password = password;
    export.zip = zip;
    export.tmpdir = tmpdir;

    return DIDStore_ListDIDs(store, 0, did_to_zip, (void*)&export);
}

static int exportprv_to_zip(DIDStore *store, const char *storepass, zip_t *zip,
        const char *password, const char *tmpdir)
{
    char tmpfile[PATH_MAX];
    zip_source_t *prv_source = NULL;

    assert(store);
    assert(storepass && *storepass);
    assert(zip);
    assert(password && *password);

    sprintf(tmpfile, "%s/prvexport.json", tmpdir);
    delete_file(tmpfile);
    if (DIDStore_ExportPrivateIdentity(store, storepass, tmpfile, password) < 0)
        goto errorExit;

    prv_source = zip_source_file(zip, tmpfile, 0, 0);
    if (!prv_source) {
        DIDError_Set(DIDERR_MALFORMED_EXPORTDID, "Get source file failed.");
        goto errorExit;
    }

    if (zip_file_add(zip, "privateIdentity", prv_source, 0) < 0) {
        DIDError_Set(DIDERR_MALFORMED_EXPORTDID, "Add source file failed.");
        goto errorExit;
    }

    return 0;
errorExit:
    if (prv_source)
        zip_source_free(prv_source);

    return -1;
}

int DIDStore_ExportStore(DIDStore *store, const char *storepass,
        const char *zipfile, const char *password)
{
    zip_t *zip = NULL;
    char tmpdir[PATH_MAX];
    int rc = -1;

    if (!store || !storepass || !*storepass || !zipfile || !*zipfile ||
            !password || !*password) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    zip = create_zip(zipfile);
    if (!zip)
        return -1;

    //create temp dir
    const char *tmp = getenv("TMPDIR");
    if (!tmp) {
        if (access("/tmp", F_OK) == 0)
            tmp = "/tmp";
        else
            return -1;
    }

    snprintf(tmpdir, sizeof(tmpdir), "%s/didexport", tmp);
    mkdirs(tmpdir, S_IRWXU);

    if (exportdid_to_zip(store, storepass, zip, password, tmpdir) < 0 ||
            exportprv_to_zip(store, storepass, zip, password, tmpdir) < 0)
        goto errorExit;

    rc = 0;

errorExit:
    if (zip)
        zip_close(zip);

    delete_file(tmpdir);
    return rc;
}

static zip_t *open_zip(const char *file)
{
    int err;
    zip_t *zip;

    assert(file && *file);

    if ((zip = zip_open(file, ZIP_RDONLY, &err)) == NULL) {
        zip_error_t error;
        zip_error_init_with_code(&error, err);
        DIDError_Set(DIDERR_MALFORMED_EXPORTDID, "Can't open zip archive '%s': %s", file, zip_error_strerror(&error));
        zip_error_fini(&error);
    }

    return zip;
}

int DIDStore_ImportStore(DIDStore *store, const char *storepass, const char *zipfile,
        const char *password)
{
    zip_t *zip = NULL;
    zip_int64_t count;
    zip_stat_t stat;
    int i, rc = -1;
    char filename[] = "/tmp/storeexport.json";

    if (!store || !storepass || !*storepass || !zipfile || !*zipfile ||
            !password || !*password) {
        DIDError_Set(DIDERR_INVALID_ARGS, "Invalid arguments.");
        return -1;
    }

    zip = open_zip(zipfile);
    if (!zip)
        return -1;

    count = zip_get_num_entries(zip, ZIP_FL_UNCHANGED);
    if (count == 0)
        goto errorExit;

    for (i = 0; i < count; i++) {
        zip_int64_t readed;
        int code;
        zip_stat_init(&stat);
        if (zip_stat_index(zip, (zip_uint64_t)i, ZIP_FL_UNCHANGED, &stat) < 0)
            goto errorExit;

        zip_file_t *zip_file = zip_fopen_index(zip, (zip_uint64_t)i, ZIP_FL_UNCHANGED);
        if (!zip_file)
            goto errorExit;

        char *buffer = (char*)malloc(stat.size + 1);
        if (!buffer) {
            zip_fclose(zip_file);
            goto errorExit;
        }

        readed = zip_fread(zip_file, buffer, stat.size);
        zip_fclose(zip_file);
        if (readed < 0)
            goto errorExit;
        buffer[stat.size] = 0;

        delete_file(filename);
        if (check_file(filename) < 0)
            goto errorExit;

        code = store_file(filename, buffer);
        free(buffer);
        if (code < 0) {
            delete_file(filename);
            goto errorExit;
        }

        if (!strcmp(stat.name, "privateIdentity")) {
            code = DIDStore_ImportPrivateIdentity(store, storepass, filename, password);
            delete_file(filename);
            if (code < 0)
                goto errorExit;
        } else {
            DID * did = DID_New(stat.name);
            if (!did)
                goto errorExit;

            code = DIDStore_ImportDID(store, storepass, filename, password);
            delete_file(filename);
            DID_Destroy(did);
            if (code < 0)
                goto errorExit;
        }
    }
    rc = 0;

errorExit:
    if (zip)
        zip_close(zip);

    return rc;
}
