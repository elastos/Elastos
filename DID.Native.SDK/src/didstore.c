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

static int store_didmeta(DIDStore *store, DIDMeta *meta, DID *did)
{
    char path[PATH_MAX];
    const char *data;
    int rc;

    assert(store);
    assert(meta);
    assert(did);

    if (DIDMeta_IsEmpty(meta))
        return 0;

    if (get_file(path, 1, 4, store->root, DID_DIR, did->idstring, META_FILE) == -1)
        return -1;

    if (test_path(path) == S_IFDIR) {
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
    if (get_file(path, 0, 4, store->root, DID_DIR, did, META_FILE) == -1)
        return 0;

    rc = test_path(path);
    if (rc < 0)
        return 0;

    if (rc == S_IFDIR) {
        delete_file(path);
        return -1;
    }

    data = load_file(path);
    if (!data)
        return -1;

    rc = DIDMeta_FromJson(meta, data);
    free((char*)data);
    if (rc)
        return rc;

    DIDMeta_SetStore(meta, store);
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
        free((char*)data);
        return -1;
    }

    if (test_path(path) == S_IFDIR) {
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
    if (rc < 0)
        return 0;

    if (rc == S_IFDIR) {
        delete_file(path);
        return -1;
    }

    data = load_file(path);
    if (!data)
        return -1;

    rc = CredentialMeta_FromJson(meta, data);
    free((char*)data);
    if (rc)
        return rc;

    CredentialMeta_SetStore(meta, store);
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
    return 0;
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

    if (encrypt_to_base64((char *)base64, storepass, extendedkey, size) == -1)
        return -1;

    if (get_file(path, 1, 3, store->root, PRIVATE_DIR, HDKEY_FILE) == -1)
        return -1;

    if (store_file(path, (const char *)base64) == -1)
        return -1;

    return 0;
}

static ssize_t load_extendedprvkey(DIDStore *store, uint8_t *extendedkey, size_t size,
        const char *storepass)
{
    const char *string;
    uint8_t *key;
    char path[PATH_MAX];
    ssize_t len;

    assert(store);
    assert(extendedkey && size >= EXTENDEDKEY_BYTES);
    assert(storepass);
    assert(*storepass);

    if (get_file(path, 0, 3, store->root, PRIVATE_DIR, HDKEY_FILE) == -1)
        return -1;

    string = load_file(path);
    if (!string)
        return -1;

    key = (uint8_t*)alloca(size * 2);
    if (!key)
        return -1;

    len = decrypt_from_base64(extendedkey, storepass, string);
    free((char*)string);

    return len;
}

static int store_extendedpubkey(DIDStore *store, uint8_t *extendedkey, size_t size)
{
    char publickeybase58[MAX_PUBKEY_BASE58];
    char path[PATH_MAX];

    assert(store);
    assert(extendedkey && size > 0);

    if (base58_encode(publickeybase58, extendedkey, size) == -1)
        return -1;

    if (get_file(path, 1, 3, store->root, PRIVATE_DIR, HDPUBKEY_FILE) == -1)
        return -1;

    if (store_file(path, (const char *)publickeybase58) == -1)
        return -1;

    return 0;
}

static ssize_t load_extendedpubkey(DIDStore *store, uint8_t *extendedkey, size_t size)
{
    const char *string;
    char path[PATH_MAX];
    ssize_t len;

    assert(store);
    assert(extendedkey && size >= EXTENDEDKEY_BYTES);

    if (get_file(path, 0, 3, store->root, PRIVATE_DIR, HDPUBKEY_FILE) == -1)
        return -1;

    string = load_file(path);
    if (!string)
        return -1;

    len = base58_decode(extendedkey, string);
    free((char*)string);

    return len;
}

static int store_mnemonic(DIDStore *store, const char *storepass, const char *mnemonic)
{
    char base64[512];
    char path[PATH_MAX];

    assert(store);
    assert(mnemonic);
    assert(*mnemonic);

    if (encrypt_to_base64(base64, storepass, (const uint8_t*)mnemonic, strlen(mnemonic)) == -1)
        return -1;

    if (get_file(path, 1, 3, store->root, PRIVATE_DIR, MNEMONIC_FILE) == -1)
        return -1;

    if (store_file(path, base64) == -1)
        return -1;

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

    if (get_file(path, 0, 3, store->root, PRIVATE_DIR, MNEMONIC_FILE) == -1)
        return -1;

    encrpted_mnemonic = load_file(path);
    if (!encrpted_mnemonic)
        return -1;

    len = decrypt_from_base64((uint8_t*)mnemonic, storepass, encrpted_mnemonic);
    free((char*)encrpted_mnemonic);

    return len;
}

static int load_index(DIDStore *store)
{
    char path[PATH_MAX];
    const char *string;
    int index;

    assert(store);

    if (get_file(path, 0, 3, store->root, PRIVATE_DIR, INDEX_FILE) == -1)
        return -1;

    string = load_file(path);
    if (!string)
        return -1;

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
    if (len < 0 || len > sizeof(string))
        return -1;

    if (get_file(path, 1, 3, store->root, PRIVATE_DIR, INDEX_FILE) == -1)
        return -1;

    if (store_file(path, string) == -1)
        return -1;

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
    if (!builder)
        return NULL;

    builder->document = (DIDDocument*)calloc(1, sizeof(DIDDocument));
    if (!builder->document) {
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

    if (init_didurl(&id, did, "primary") == -1)
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

    if (DIDMeta_Init(&document->meta, alias, NULL, false, 0) == -1) {
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
    return -1;
}

/////////////////////////////////////////////////////////////////////////
DIDStore* DIDStore_Open(const char *root, DIDAdapter *adapter)
{
    char path[PATH_MAX];
    DIDStore *didstore;

    if (!root || !*root || strlen(root) >= PATH_MAX|| !adapter)
        return NULL;

    didstore = (DIDStore *)calloc(1, sizeof(DIDStore));
    if (!didstore)
        return NULL;

    strcpy(didstore->root, root);
    didstore->backend.adapter = adapter;

    if (get_dir(path, 0, 1, root) == 0 && !check_store(didstore))
        return didstore;

    if (get_dir(path, 1, 1, root) == 0 && !create_store(didstore))
        return didstore;

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
    if (!store || !storepass || !*storepass || !mnemonic || size <= 0)
        return -1;

    return load_mnemonic(store, storepass, mnemonic, size);
}

int DIDStore_StoreDID(DIDStore *store, DIDDocument *document, const char *alias)
{
    char path[PATH_MAX];
    const char *data, *root;
    DIDMeta meta;
    ssize_t count;
    int rc;

    if (!store || !document)
        return -1;

    if (load_didmeta(store, &meta, document->did.idstring) == -1 ||
            DIDMeta_SetAlias(&document->meta, alias) == -1 ||
            DIDMeta_Merge(&meta, &document->meta) == -1)
        return -1;

    memcpy(&document->meta, &meta, sizeof(DIDMeta));
    DIDDocument_SetStore(document, store);

	data = DIDDocument_ToJson(document, true);
	if (!data)
		return -1;

    rc = get_file(path, 1, 4, store->root, DID_DIR, document->did.idstring, DOCUMENT_FILE);
    if (rc < 0) {
        free((char*)data);
        return -1;
    }

    rc = store_file(path, data);
    free((char*)data);
    if (rc)
        goto errorExit;

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

    if (!store || !did)
        return NULL;

    if (get_file(path, 0, 4, store->root, DID_DIR, did->idstring, DOCUMENT_FILE) == -1)
        return NULL;

    rc = test_path(path);
    if (rc < 0)
        return NULL;

    if (rc == S_IFDIR) {
        delete_file(path);
        return NULL;
    }

    data = load_file(path);
    if (!data)
        return NULL;

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

    if (!store || !did)
        return false;

    if (get_dir(path, 0, 3, store->root, DID_DIR, did->idstring) == -1)
        return false;

    rc = test_path(path);
    if (rc < 0)
        return false;

    if (rc == S_IFREG || is_empty(path)) {
        delete_file(path);
        return false;
    }

    return true;
}

bool DIDStore_DeleteDID(DIDStore *store, DID *did)
{
    char path[PATH_MAX];
    int rc;

    if (!store || !did)
        return false;

    if (get_dir(path, 0, 3, store->root, DID_DIR, did->idstring) == -1)
        return false;

    if (test_path(path) > 0) {
        delete_file(path);
        return true;
    } else {
        return false;
    }
}

int DIDStore_ListDIDs(DIDStore *store, ELA_DID_FILTER filter,
        DIDStore_DIDsCallback *callback, void *context)
{
    char path[PATH_MAX];
    DID_List_Helper dh;
    int rc;

    if (!store || !callback)
        return -1;

    if (get_dir(path, 0, 2, store->root, DID_DIR) == -1)
        return -1;

    rc = test_path(path);
    if (rc < 0)
        return -1;

    if (rc != S_IFDIR)
        return -1;

    dh.store = store;
    dh.cb = callback;
    dh.context = context;
    dh.filter = filter;

    if (list_dir(path, "*", list_did_helper, (void*)&dh) == -1)
        return -1;

    return 0;
}

int DIDStore_StoreCredential(DIDStore *store, Credential *credential, const char *alias)
{
    CredentialMeta meta;
    DIDURL *id;

    if (!store || !credential)
        return -1;

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
            CREDENTIALS_DIR, id->fragment, CREDENTIAL_FILE) == -1)
        return NULL;

    rc = test_path(path);
    if (rc < 0)
        return NULL;

    if (rc == S_IFDIR) {
        delete_file(path);
        return NULL;
    }

    data = load_file(path);
    if (!data)
        return NULL;

    credential = Credential_FromJson(data, did);
    free((char*)data);
    if (!credential)
        return NULL;

    if (didstore_loadcredmeta(store, &credential->meta, id) == -1) {
        Credential_Destroy(credential);
        return NULL;
    }
    return credential;
}

bool DIDStore_ContainsCredentials(DIDStore *store, DID *did)
{
    char path[PATH_MAX];
    int rc;

    if (!store || !did)
        return false;

    if (get_dir(path, 0, 4, store->root, DID_DIR, did->idstring, CREDENTIALS_DIR) == -1)
        return -1;

    rc = test_path(path);
    if (rc < 0)
        return false;

    if (rc == S_IFREG) {
        delete_file(path);
        return false;
    }

    return !is_empty(path);
}

bool DIDStore_ContainsCredential(DIDStore *store, DID *did, DIDURL *id)
{
    char path[PATH_MAX];
    int rc;

    if (!store || !did || !id)
        return false;

    if (get_dir(path, 0, 5, store->root, DID_DIR, did->idstring,
            CREDENTIALS_DIR, id->fragment) == -1)
        return false;

    rc = test_path(path);
    if (rc < 0)
        return false;

    if (rc == S_IFREG) {
        delete_file(path);
        return false;
    }

    return true;
}

bool DIDStore_DeleteCredential(DIDStore *store, DID *did, DIDURL *id)
{
    char path[PATH_MAX];

    if (!store || !did || !id)
        return false;

    if (get_dir(path, 0, 5, store->root, DID_DIR, did->idstring,
            CREDENTIALS_DIR, id->fragment) == -1)
        return false;

    if (is_empty(path))
        return false;

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

    if (!store || !did || !callback)
        return -1;

    if (get_dir(path, 0, 4, store->root, DID_DIR, did->idstring, CREDENTIALS_DIR) == -1)
        return -1;

    rc = test_path(path);
    if (rc < 0)
        return -1;

    if (rc == S_IFREG) {
        delete_file(path);
        return -1;
    }

    ch.store = store;
    ch.cb = callback;
    ch.context = context;
    strcpy((char*)ch.did.idstring, did->idstring);
    ch.type = NULL;

    if (list_dir(path, "*", list_credential_helper, (void*)&ch) == -1)
        return -1;

    return 0;
}

int DIDStore_SelectCredentials(DIDStore *store, DID *did, DIDURL *id,
        const char *type, DIDStore_CredentialsCallback *callback, void *context)
{
    char path[PATH_MAX];
    Cred_List_Helper ch;
    int rc;

    if (!store || !did || (!id && !type) || !callback)
        return -1;

    if (id) {
        if (get_file(path, 0, 6, store->root, DID_DIR, did->idstring,
                CREDENTIALS_DIR, id->fragment, CREDENTIAL_FILE) == -1)
            return -1;

        if (test_path(path) > 0) {
            if ((type && has_type(did, path, type) == true) || !type)
                return callback(id, context);
            return -1;
        }
        return -1;
    }

    if (get_dir(path, 0, 4, store->root, DID_DIR, did->idstring, CREDENTIALS_DIR) == -1)
        return -1;

    rc = test_path(path);
    if (rc < 0)
        return -1;

    if (rc == S_IFREG) {
        delete_file(path);
        return -1;
    }

    ch.store = store;
    ch.cb = callback;
    ch.context = context;
    strcpy((char*)ch.did.idstring, did->idstring);
    ch.type = type;

    if (list_dir(path, "*.*", select_credential_helper, (void*)&ch) == -1)
        return -1;

    return 0;
}

bool DIDSotre_ContainsPrivateKeys(DIDStore *store, DID *did)
{
    char path[PATH_MAX];

    if (!store || !did)
        return false;

    if (get_dir(path, 0, 4, store->root, DID_DIR, did->idstring, PRIVATEKEYS_DIR) == -1)
        return -1;

    return !is_empty(path);
}

bool DIDStore_ContainsPrivateKey(DIDStore *store, DID *did, DIDURL *id)
{
    char path[PATH_MAX];
    int rc;

    if (!store || !did || !id)
        return false;

    if (get_file(path, 0, 5, store->root, DID_DIR, did->idstring,
            PRIVATEKEYS_DIR, id->fragment) == -1)
        return false;

    rc = test_path(path);
    if (rc < 0)
        return false;

    if (rc == S_IFDIR) {
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

    if (!store || !storepass || !*storepass || !did || !id || !privatekey)
        return -1;

    if (!DID_Equals(DIDURL_GetDid(id), did))
        return -1;

    if (encrypt_to_base64(base64, storepass, privatekey, PRIVATEKEY_BYTES) == -1)
        return -1;

    if (get_file(path, 1, 5, store->root, DID_DIR, did->idstring,
            PRIVATEKEYS_DIR, id->fragment) == -1)
        return -1;

    if (!store_file(path, base64))
        return 0;

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
        if (!HDKey_FromExtendedKey(extendedkey, sizeof(extendedkey), identity))
            rc = -1;
    }

    memset(extendedkey, 0, sizeof(extendedkey));
    return rc == 0 ? identity : NULL;
}

static HDKey *load_publicIdentity(DIDStore *store, HDKey *identity)
{
    uint8_t extendedkey[EXTENDEDKEY_BYTES];
    ssize_t size;
    int rc = 0;

    assert(store);
    assert(identity);

    size = load_extendedpubkey(store, extendedkey, sizeof(extendedkey));
    if (size == -1)
        return NULL;

    return HDKey_FromExtendedKey(extendedkey, sizeof(extendedkey), identity);
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

    if (init_did(&did, idstring) == -1 || init_didurl(&id, &did, "primary") == -1)
        return -1;

    if (DIDStore_StorePrivateKey(store, storepass, &did, &id, (unsigned char *)privatekey) == -1)
        return -1;

    return 0;
}

int DIDStore_Synchronize(DIDStore *store, const char *storepass)
{
    int rc, nextindex, i = 0, blanks = 0;
    DIDDocument *document;
    HDKey _identity, *privateIdentity;
    DerivedKey _derivedkey, *derivedkey;
    DID did;

    if (!store || !storepass || !*storepass)
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

        if (init_did(&did, DerivedKey_GetAddress(derivedkey)) == 0) {
            document = DID_Resolve(&did, true);
            if (document) {
                if (DIDStore_StoreDID(store, document, "") == 0) {
                    if (store_default_privatekey(store, storepass,
                            DerivedKey_GetAddress(derivedkey),
                            DerivedKey_GetPrivateKey(derivedkey)) == 0) {
                        if (store_index(store, i) == -1)
                            DIDStore_DeleteDID(store, &did);

                    } else {
                        DIDStore_DeleteDID(store, &did);
                    }

                } else {
                    blanks = 0;
                }
                DIDDocument_Destroy(document);
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

    if (!store)
        return false;

    if (get_file(path, 0, 3, store->root, PRIVATE_DIR, HDKEY_FILE) == -1)
        return false;

    if (stat(path, &st) < 0)
        return false;

    return st.st_size > 0;
}

int DIDStore_InitPrivateIdentity(DIDStore *store, const char *storepass,
        const char *mnemonic, const char *passphrase, const char *language, bool force)
{
    uint8_t extendedkey[EXTENDEDKEY_BYTES];
    char path[PATH_MAX];
    ssize_t size;
    HDKey _privateIdentity, *privateIdentity;

    if (!store || !storepass || !*storepass|| !mnemonic || !*mnemonic)
        return -1;

    if (!passphrase)
        passphrase = "";

    //check if DIDStore has existed private identity
    if (get_file(path, 0, 3, store->root, PRIVATE_DIR, HDKEY_FILE) == 0) {
        if (DIDStore_ContainsPrivateIdentity(store) && !force)
            return -1;
    }

    privateIdentity = HDKey_FromMnemonic(mnemonic, passphrase, language, &_privateIdentity);
    if (!privateIdentity)
        return -1;

    size = HDKey_SerializePrv(privateIdentity, extendedkey, sizeof(extendedkey));
    if (size == -1)
        return -1;

    if (store_extendedprvkey(store, extendedkey, size, storepass) == -1)
        return -1;

    memset(extendedkey, 0, sizeof(extendedkey));
    size = HDKey_SerializePub(privateIdentity, extendedkey, sizeof(extendedkey));
    if (size == -1)
        return -1;

    if (store_extendedpubkey(store, extendedkey, size) == -1)
        return -1;

    if (store_mnemonic(store, storepass, mnemonic) == -1)
        return -1;

    return store_index(store, 0);
}

int DIDStore_InitPrivateIdentityFromRootKey(DIDStore *store, const char *storepass,
        const char *extendedprvkey, bool force)
{
    uint8_t seed[SEED_BYTES], extendedkey[EXTENDEDKEY_BYTES];
    char path[PATH_MAX];
    ssize_t size;
    HDKey _privateIdentity, *privateIdentity;

    if (!store || !storepass || !*storepass || !extendedprvkey)
        return -1;

    //check if DIDStore has existed private identity
    if (get_file(path, 0, 3, store->root, PRIVATE_DIR, HDKEY_FILE) == 0) {
        if (DIDStore_ContainsPrivateIdentity(store) && !force)
            return -1;
    }

    size = base58_decode(extendedkey, extendedprvkey);
    if (size == -1)
        return -1;

    privateIdentity = HDKey_FromExtendedKey(extendedkey, size, &_privateIdentity);
    if (!privateIdentity)
        return -1;

    size = HDKey_SerializePrv(privateIdentity, extendedkey, sizeof(extendedkey));
    if (size == -1)
        return -1;

    if (store_extendedprvkey(store, extendedkey, size, storepass) == -1)
        return -1;

    memset(extendedkey, 0, sizeof(extendedkey));
    size = HDKey_SerializePub(privateIdentity, extendedkey, sizeof(extendedkey));
    if (size == -1)
        return -1;

    if (store_extendedpubkey(store, extendedkey, size) == -1)
        return -1;

    return store_index(store, 0);
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
    return dkey;
}

DIDDocument *DIDStore_NewDID(DIDStore *store, const char *storepass, const char *alias)
{
    int index;
    DIDDocument *document;

    if (!store || !storepass || !*storepass)
        return NULL;

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

    if (!store || index < 0)
        return NULL;

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

    if (!store || !storepass || !*storepass || index < 0)
        return NULL;

    derivedkey = get_childkey(store, storepass, index, &_derivedkey);
    if (!derivedkey)
        return NULL;

    if (init_did(&did, DerivedKey_GetAddress(derivedkey)) == -1) {
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

    base58_encode(publickeybase58, DerivedKey_GetPublicKey(derivedkey), PUBLICKEY_BYTES);
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

int DIDStore_Signv(DIDStore *store, const char *storepass, DID *did, DIDURL *key,
        char *sig, int count, va_list inputs)
{
    const char *privatekey;
    unsigned char binkey[PRIVATEKEY_BYTES];
    char path[PATH_MAX];
    int rc;

    if (!store || !storepass || !*storepass || !did || !key || !sig || count <= 0)
        return -1;

    if (get_file(path, 0, 5, store->root, DID_DIR, did->idstring, PRIVATEKEYS_DIR, key->fragment) == -1)
        return -1;

    privatekey = load_file(path);
    if (!privatekey)
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

int DIDStore_Sign(DIDStore *store, const char *storepass, DID *did, DIDURL *key,
        char *sig, int count, ...)
{
    int rc;
    va_list inputs;

    if (!store || !storepass || !*storepass || !did || !key || !sig || count <= 0)
        return -1;

    va_start(inputs, count);
    rc = DIDStore_Signv(store, storepass, did, key, sig, count, inputs);
    va_end(inputs);

    return rc;
}

const char *DIDStore_PublishDID(DIDStore *store, const char *storepass, DID *did,
        DIDURL *signKey, bool force)
{
    const char *txid, *resolvetxid;
    DIDDocument *doc, *resolvedoc;
    int rc = -1;

    if (!store || !storepass || !*storepass || !did)
        return NULL;

    doc = DIDStore_LoadDID(store, did);
    if (!doc)
        return NULL;

    if (DIDDocument_IsDeactivated(doc) || (!force && DIDDocument_IsExpires(doc))) {
        DIDDocument_Destroy(doc);
        return NULL;
    }

    if (!signKey)
        signKey = DIDDocument_GetDefaultPublicKey(doc);

    resolvedoc = DID_Resolve(did, true);
    if (!resolvedoc) {
        txid = DIDBackend_Create(&store->backend, doc, signKey, storepass);
    } else {
        resolvetxid = DIDDocument_GetTxid(resolvedoc);
        if (!*resolvetxid)
            goto errorExit;

        if (force) {
            DIDMeta_SetTxid(&doc->did.meta, resolvetxid);
            DIDMeta_SetTxid(&doc->meta, resolvetxid);
        } else {
            if (DIDDocument_IsDeactivated(resolvedoc))
                goto errorExit;

            txid = DIDDocument_GetTxid(doc);
            if (!txid || strcmp(txid, resolvetxid))
                goto errorExit;
        }
        txid = DIDBackend_Update(&store->backend, doc, signKey, storepass);
    }

    if (!txid || !*txid)
        goto errorExit;

    DIDMeta_SetTxid(&doc->meta, txid);
    rc = store_didmeta(store, &doc->meta, &doc->did);

errorExit:
    if (resolvedoc)
        DIDDocument_Destroy(resolvedoc);
    if (doc)
        DIDDocument_Destroy(doc);
    return rc == -1 ? NULL : txid;
}

const char *DIDStore_DeactivateDID(DIDStore *store, const char *storepass,
        DID *did, DIDURL *signKey)
{
    const char *txid;
    DIDDocument *doc;

    if (!store || !storepass || !*storepass || !did || !signKey)
        return NULL;

    if (!signKey) {
        doc = DID_Resolve(did, false);
        if (!doc || DIDStore_StoreDID(store, doc, "") == -1)
            return NULL;

        signKey = DIDDocument_GetDefaultPublicKey(doc);
        if (!signKey)
            return NULL;
    }

    txid = DIDBackend_Deactivate(&store->backend, did, signKey, storepass);
    DIDDocument_Destroy(doc);
    return txid;
}
