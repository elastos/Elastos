#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#ifdef HAVE_IO_H
#include <io.h>
#endif
#ifdef HAVE_GLOB_H
#include <glob.h>
#endif
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <pwd.h>

#include "ela_did.h"
#include "dummyadapter.h"
#include "constant.h"
#include "loader.h"
#include "didtest_adapter.h"
#include "crypto.h"
#include "HDkey.h"
#include "did.h"
#include "diddocument.h"
#include "didstore.h"
#include "credential.h"
#include "credmeta.h"

typedef struct TestData {
    DIDStore *store;

    DIDDocument *issuerdoc;
    char *issuerJson;
    char *issuerCompactJson;
    char *issuerNormalizedJson;

    DIDDocument *doc;
    char *docJson;
    char *docCompactJson;
    char *docNormalizedJson;

    Credential *profileVc;
    char *profileVcCompactJson;
    char *profileVcNormalizedJson;

    Credential *emailVc;
    char *emailVcCompactJson;
    char *emailVcNormalizedJson;

    Credential *passportVc;
    char *passportVcCompactJson;
    char *passportVcNormalizedJson;

    Credential *twitterVc;
    char *twitterVcCompactJson;
    char *twitterVcNormalizedJson;

    Credential *Vc;
    char *VcCompactJson;
    char *VcNormalizedJson;

    Presentation *vp;
    char *vpNormalizedJson;

    char *restoreMnemonic;
} TestData;

TestData testdata;

static DIDAdapter *adapter;
static DummyAdapter *dummyadapter;

char *get_wallet_path(char* path, const char* dir)
{
    if (!path || !dir)
        return NULL;

    sprintf(path, "%s/%s", getenv("HOME"), dir);
    return path;
}

const char *get_store_path(char* path, const char *dir)
{
    if (!path || !dir)
        return NULL;

    if(!getcwd(path, PATH_MAX)) {
        printf("\nCan't get current dir.");
        return NULL;
    }

    strcat(path, dir);
    return path;
}

char *get_path(char *path, const char *file)
{
    size_t len;

    assert(file);
    assert(*file);

    len = snprintf(path, PATH_MAX, "../etc/did/resources/testdata/%s", file);
        if (len < 0 || len > PATH_MAX)
            return NULL;

    return path;
}

static char *load_file(const char *file)
{
    char _path[PATH_MAX];
    char *readstring = NULL, *path;
    size_t reclen, bufferlen;
    struct stat st;
    int fd;

    assert(file);
    assert(*file);

    path = get_path(_path, file);
    if (!path)
        return NULL;

    fd = open(path, O_RDONLY);
    if (fd == -1)
        return NULL;

    if (fstat(fd, &st) < 0) {
        close(fd);
        return NULL;
    }

    bufferlen = st.st_size;
    readstring = calloc(1, bufferlen + 1);
    if (!readstring)
        return NULL;

    reclen = read(fd, readstring, bufferlen);
    if(reclen == 0 || reclen == -1)
        return NULL;

    close(fd);
    return readstring;
}

static const char *getpassword(const char *walletDir, const char *walletId)
{
    return walletpass;
}

char *get_file_path(char *path, size_t size, int count, ...)
{
    va_list list;
    int totalsize = 0;

    if (!path || size <= 0 || count <= 0)
        return NULL;

    *path = 0;
    va_start(list, count);
    for (int i = 0; i < count; i++) {
        const char *suffix = va_arg(list, const char*);
        assert(suffix);
        int len = strlen(suffix);
        totalsize = totalsize + len;
        if (totalsize > size)
            return NULL;

        strncat(path, suffix, len + 1);
    }
    va_end(list);

    return path;
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

void delete_file(const char *path);

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

void delete_file(const char *path)
{
    int rc;

    if (!path)
        return;

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

static Credential *store_credential(const char *file, const char *alias)
{
    Credential *cred;
    const char *data;
    DIDStore *store;

    data = load_file(file);
    if (!data)
        return NULL;

    cred = Credential_FromJson(data, NULL);
    free((char*)data);
    if (!cred)
        return NULL;

    if (DIDStore_StoreCredential(testdata.store, cred, alias) == -1) {
        Credential_Destroy(cred);
        return NULL;
    }

    return cred;
}

static DIDDocument *store_document(const char *file, const char *alias)
{
    DIDDocument *doc;
    const char *string;
    DIDStore *store;

    string = load_file(file);
    if (!string)
        return NULL;

    doc = DIDDocument_FromJson(string);
    free((char*)string);
    if (!doc)
        return NULL;

    if (DIDStore_StoreDID(testdata.store, doc, alias) == -1) {
        DIDDocument_Destroy(doc);
        return NULL;
    }

    return doc;
}

bool file_exist(const char *path)
{
    return test_path(path) == S_IFREG;
}

bool dir_exist(const char* path)
{
    return test_path(path) == S_IFDIR;
}

static int import_privatekey(DIDURL *id, const char *storepass, const char *file)
{
    char *skbase;
    DIDStore *store;
    uint8_t privatekey[PRIVATEKEY_BYTES];

    if (!id || !file || !*file)
        return -1;

    skbase = load_file(file);
    if (!skbase || !*skbase)
        return -1;

    if (base58_decode(privatekey, sizeof(privatekey), skbase) != PRIVATEKEY_BYTES) {
        free(skbase);
        return -1;
    }

    free(skbase);
    if (DIDStore_StorePrivateKey(testdata.store, storepass, DIDURL_GetDid(id), id, privatekey) == -1)
        return -1;

    return 0;
}

DID *DID_Copy(DID *dest, DID *src)
{
    if (!dest || !src) {
        return NULL;
    }

    strcpy(dest->idstring, src->idstring);
    memcpy(&dest->meta, &src->meta, sizeof(DIDMeta));
    return dest;
}

DIDURL *DIDURL_Copy(DIDURL *dest, DIDURL *src)
{
    if (!dest || !src ) {
        return NULL;
    }

    strcpy(dest->did.idstring, src->did.idstring);
    strcpy(dest->fragment, src->fragment);
    memcpy(&dest->meta, &src->meta, sizeof(CredentialMeta));

    return dest;
}

/////////////////////////////////////
void TestData_Init(void)
{
    char _dir[PATH_MAX];
    char *walletDir;

    walletDir = get_wallet_path(_dir, walletdir);
    adapter = TestDIDAdapter_Create(walletDir, walletId, network, getpassword);
    dummyadapter = DummyAdapter_Create();
}

void TestData_Deinit(void)
{
    TestDIDAdapter_Destroy(adapter);
    DummyAdapter_Destroy();
}

DIDAdapter *TestData_GetAdapter(bool dummybackend)
{
    if (dummybackend)
        return &dummyadapter->adapter;

    return adapter;
}

DIDStore *TestData_SetupStore(bool dummybackend, const char *root)
{
    char cachedir[PATH_MAX];

    if (!root || !*root)
        return NULL;

    sprintf(cachedir, "%s%s", getenv("HOME"), "/.cache.did.elastos");

    delete_file(root);
    if (dummybackend) {
        dummyadapter->reset(dummyadapter);
        testdata.store = DIDStore_Open(root, &dummyadapter->adapter);
        DIDBackend_Initialize(&dummyadapter->resolver, cachedir);
    } else {
        testdata.store = DIDStore_Open(root, adapter);
        DIDBackend_InitializeDefault(resolver, cachedir);
    }
    return testdata.store;
}

int TestData_InitIdentity(DIDStore *store)
{
    const char *mnemonic;
    int rc;

    mnemonic = Mnemonic_Generate(language);
    rc = DIDStore_InitPrivateIdentity(store, storepass, mnemonic, passphase, language, false);
    Mnemonic_Free((void*)mnemonic);

    return rc;
}

const char *TestData_LoadIssuerJson(void)
{
    if (!testdata.issuerJson)
        testdata.issuerJson = load_file("issuer.json");

    return testdata.issuerJson;
}

const char *TestData_LoadIssuerCompJson(void)
{
    if (!testdata.issuerCompactJson)
        testdata.issuerCompactJson = load_file("issuer.compact.json");

    return testdata.issuerCompactJson;
}

const char *TestData_LoadIssuerNormJson(void)
{
    if (!testdata.issuerNormalizedJson)
        testdata.issuerNormalizedJson = load_file("issuer.normalized.json");

    return testdata.issuerNormalizedJson;
}

const char *TestData_LoadDocJson(void)
{
    if (!testdata.docJson)
        testdata.docJson = load_file("document.json");

    return testdata.docJson;
}

const char *TestData_LoadDocCompJson(void)
{
    if (!testdata.docCompactJson)
        testdata.docCompactJson = load_file("document.compact.json");

    return testdata.docCompactJson;
}

const char *TestData_LoadDocNormJson(void)
{
    if (!testdata.docNormalizedJson)
        testdata.docNormalizedJson = load_file("document.normalized.json");

    return testdata.docNormalizedJson;
}

Credential *TestData_LoadProfileVc(void)
{
    if (!testdata.profileVc)
        testdata.profileVc = store_credential("vc-profile.json", "profile vc");

    return testdata.profileVc;
}

const char *TestData_LoadProfileVcCompJson(void)
{
    if (!testdata.profileVcCompactJson)
        testdata.profileVcCompactJson = load_file("vc-profile.compact.json");

    return testdata.profileVcCompactJson;
}

const char *TestData_LoadProfileVcNormJson(void)
{
    if (!testdata.profileVcNormalizedJson)
        testdata.profileVcNormalizedJson = load_file("vc-profile.normalized.json");

    return testdata.profileVcNormalizedJson;
}

Credential *TestData_LoadEmailVc(void)
{
    if (!testdata.emailVc)
        testdata.emailVc = store_credential("vc-email.json", "email vc");

    return testdata.emailVc;
}

const char *TestData_LoadEmailVcCompJson(void)
{
    if (!testdata.emailVcCompactJson)
        testdata.emailVcCompactJson = load_file("vc-email.compact.json");

    return testdata.emailVcCompactJson;
}

const char *TestData_LoadEmailVcNormJson(void)
{
    if (!testdata.emailVcNormalizedJson)
        testdata.emailVcNormalizedJson = load_file("vc-email.normalized.json");

    return testdata.emailVcNormalizedJson;
}

Credential *TestData_LoadPassportVc(void)
{
    if (!testdata.passportVc)
        testdata.passportVc = store_credential("vc-passport.json", "passport vc");

    return testdata.passportVc;
}

const char *TestData_LoadPassportVcCompJson(void)
{
    if (!testdata.passportVcCompactJson)
        testdata.passportVcCompactJson = load_file("vc-passport.compact.json");

    return testdata.passportVcCompactJson;
}

const char *TestData_LoadPassportVcNormJson(void)
{
    if (!testdata.passportVcNormalizedJson)
        testdata.passportVcNormalizedJson = load_file("vc-passport.normalized.json");

    return testdata.passportVcNormalizedJson;
}

Credential *TestData_LoadTwitterVc(void)
{
    if (!testdata.twitterVc)
        testdata.twitterVc = store_credential("vc-twitter.json", "twitter vc");

    return testdata.twitterVc;
}

const char *TestData_LoadTwitterVcCompJson(void)
{
    if (!testdata.twitterVcCompactJson)
        testdata.twitterVcCompactJson = load_file("vc-twitter.compact.json");

    return testdata.twitterVcCompactJson;
}

const char *TestData_LoadTwitterVcNormJson(void)
{
    if (!testdata.twitterVcNormalizedJson)
        testdata.twitterVcNormalizedJson = load_file("vc-twitter.normalized.json");

    return testdata.twitterVcNormalizedJson;
}

Credential *TestData_LoadVc(void)
{
    if (!testdata.Vc)
        testdata.Vc = store_credential("vc-json.json", "test vc");

    return testdata.Vc;
}

const char *TestData_LoadVcCompJson(void)
{
    if (!testdata.VcCompactJson)
        testdata.VcCompactJson = load_file("vc-json.compact.json");

    return testdata.VcCompactJson;
}

const char *TestData_LoadVcNormJson(void)
{
    if (!testdata.VcNormalizedJson)
        testdata.VcNormalizedJson = load_file("vc-json.normalized.json");

    return testdata.VcNormalizedJson;
}

Presentation *TestData_LoadVp(void)
{
    const char *data = load_file("vp.json");
    if (!data)
        return NULL;

    testdata.vp = Presentation_FromJson(data);
    free((char*)data);
    return testdata.vp;
}

const char *TestData_LoadVpNormJson(void)
{
    if (!testdata.vpNormalizedJson)
        testdata.vpNormalizedJson = load_file("vp.normalized.json");

    return testdata.vpNormalizedJson;
}

DIDDocument *TestData_LoadDoc(void)
{
    DIDURL *id;
    DID *subject;
    int rc;

    if (!testdata.doc)
       testdata.doc = store_document("document.json", "doc test");

    subject = DIDDocument_GetSubject(testdata.doc);
    id = DIDURL_NewByDid(subject, "key2");
    rc = import_privatekey(id, storepass, "document.key2.sk");
    DIDURL_Destroy(id);
    if (rc)
        return NULL;

    id = DIDURL_NewByDid(subject, "key3");
    rc = import_privatekey(id, storepass, "document.key3.sk");
    DIDURL_Destroy(id);
    if (rc)
        return NULL;

    id = DIDURL_NewByDid(subject, "primary");
    rc = import_privatekey(id, storepass, "document.primary.sk");
    DIDURL_Destroy(id);
    if (rc)
        return NULL;

    if (!DID_Resolve(subject, true) &&
            !DIDStore_PublishDID(testdata.store, storepass, subject, NULL, false))
        return NULL;

    return testdata.doc;
}

DIDDocument *TestData_LoadIssuerDoc(void)
{
    DIDURL *id;
    DID *subject;
    int rc;

    if (!testdata.issuerdoc)
        testdata.issuerdoc = store_document("issuer.json", "issuer test");

    subject = DIDDocument_GetSubject(testdata.issuerdoc);
    id = DIDURL_NewByDid(subject, "primary");
    rc = import_privatekey(id, storepass, "issuer.primary.sk");
    DIDURL_Destroy(id);
    if (rc)
        return NULL;

    if (!DID_Resolve(subject, true) &&
            !DIDStore_PublishDID(testdata.store, storepass, subject, NULL, false))
        return NULL;

    return testdata.issuerdoc;
}

const char *TestData_LoadRestoreMnemonic(void)
{
    if (!testdata.restoreMnemonic)
        testdata.restoreMnemonic = load_file("mnemonic.restore");

    return testdata.restoreMnemonic;
}

void TestData_Free(void)
{
    DIDStore_Close(testdata.store);

    if (testdata.issuerdoc)
        DIDDocument_Destroy(testdata.issuerdoc);
    if (testdata.issuerJson)
        free(testdata.issuerJson);
    if (testdata.issuerCompactJson)
        free(testdata.issuerCompactJson);
    if (testdata.issuerNormalizedJson)
        free(testdata.issuerNormalizedJson);

    if (testdata.doc)
        DIDDocument_Destroy(testdata.doc);
    if (testdata.docJson)
        free(testdata.docJson);
    if (testdata.docCompactJson)
        free(testdata.docCompactJson);
    if (testdata.docNormalizedJson)
        free(testdata.docNormalizedJson);

    if (testdata.profileVc)
        Credential_Destroy(testdata.profileVc);
    if (testdata.profileVcCompactJson)
        free(testdata.profileVcCompactJson);
    if (testdata.profileVcNormalizedJson)
        free(testdata.profileVcNormalizedJson);

    if (testdata.emailVc)
        Credential_Destroy(testdata.emailVc);
    if (testdata.emailVcCompactJson)
        free(testdata.emailVcCompactJson);
    if (testdata.emailVcNormalizedJson)
        free(testdata.emailVcNormalizedJson);

    if (testdata.passportVc)
        Credential_Destroy(testdata.passportVc);
    if (testdata.passportVcCompactJson)
        free(testdata.passportVcCompactJson);
    if (testdata.passportVcNormalizedJson)
        free(testdata.passportVcNormalizedJson);

    if (testdata.twitterVc)
        Credential_Destroy(testdata.twitterVc);
    if (testdata.twitterVcCompactJson)
        free(testdata.twitterVcCompactJson);
    if (testdata.twitterVcNormalizedJson)
        free(testdata.twitterVcNormalizedJson);

    if (testdata.Vc)
        Credential_Destroy(testdata.Vc);
    if (testdata.VcCompactJson)
        free(testdata.VcCompactJson);
    if (testdata.VcNormalizedJson)
        free(testdata.VcNormalizedJson);

    if (testdata.vp)
        Presentation_Destroy(testdata.vp);
    if (testdata.vpNormalizedJson)
        free(testdata.vpNormalizedJson);

    if (testdata.restoreMnemonic)
        free(testdata.restoreMnemonic);

    memset(&testdata, 0, sizeof(testdata));
}

/////////////////////////////////////////
const char *Generater_Publickey(char *publickeybase58, size_t size)
{
    const char *mnemonic;
    uint8_t extendedkey[EXTENDEDKEY_BYTES];
    uint8_t publickey[PUBLICKEY_BYTES];
    HDKey hk, *privateIdentity;
    DerivedKey _derivedkey, *derivedkey;
    ssize_t len;

    if (size < MAX_PUBLICKEY_BASE58)
        return NULL;

    mnemonic = Mnemonic_Generate(language);
    if (!mnemonic || !*mnemonic)
        return NULL;

    privateIdentity = HDKey_FromMnemonic(mnemonic, "", language, &hk);
    Mnemonic_Free((char*)mnemonic);
    if (!privateIdentity)
        return NULL;

    derivedkey = HDKey_GetDerivedKey(privateIdentity, 0, &_derivedkey);
    if (!derivedkey)
        return NULL;

    return DerivedKey_GetPublicKeyBase58(derivedkey, publickeybase58, size);
}

DerivedKey *Generater_KeyPair(DerivedKey *dkey)
{
    const char *mnemonic;
    uint8_t extendedkey[EXTENDEDKEY_BYTES];
    HDKey hk, *privateIdentity;
    ssize_t size;

    mnemonic = Mnemonic_Generate(language);
    if (!mnemonic || !*mnemonic)
        return NULL;

    privateIdentity = HDKey_FromMnemonic(mnemonic, "", language, &hk);
    Mnemonic_Free((char*)mnemonic);
    if (!privateIdentity)
        return NULL;

    return HDKey_GetDerivedKey(privateIdentity, 0, dkey);
}

int Set_Doc_Txid(DIDDocument *doc, const char *txid)
{
    if (!doc)
        return -1;

    if (txid) {
        if (strlen(txid) >= sizeof(doc->meta.txid))
            return -1;
        strcpy(doc->meta.txid, txid);
    } else {
        *doc->meta.txid = 0;
    }

    memcpy(&doc->did.meta, &doc->meta, sizeof(doc->meta));
    return 0;
}

int Set_Doc_Signature(DIDDocument *doc, const char *signature)
{
    if (!doc)
        return -1;

    if (signature) {
        if (strlen(signature) >= sizeof(doc->meta.signatureValue))
            return -1;
        strcpy(doc->meta.signatureValue, signature);
    } else {
        *doc->meta.signatureValue = 0;
    }

    memcpy(&doc->did.meta, &doc->meta, sizeof(doc->meta));
    return 0;
}

