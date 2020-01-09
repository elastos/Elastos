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

#include "constant.h"
#include "loader.h"
#include "didtest_adapter.h"
#include "crypto.h"
#include "HDkey.h"
#include "did.h"
#include "diddocument.h"
#include "didstore.h"

typedef struct TestData {
    DIDStore *store;
    DIDAdapter *adapter;

    DIDDocument *issuerdoc;
    char *issuerJson;
    char *issuerCompactJson;
    char *issuerNormalizedJson;

    DIDDocument *doc;
    char *docJson;
    char *docCompactJson;
    char *docNormalizedJson;

    char *profileVcJson;
    char *profileVcCompactJson;
    char *profileVcNormalizedJson;

    char *emailVcJson;
    char *emailVcCompactJson;
    char *emailVcNormalizedJson;

    char *passportVcJson;
    char *passportVcCompactJson;
    char *passportVcNormalizedJson;

    char *twitterVcJson;
    char *twitterVcCompactJson;
    char *twitterVcNormalizedJson;

    char *vpJson;
    char *vpNormalizedJson;
    char *restoreMnemonic;
} TestData;

TestData testdata;

char *get_wallet_path(char* path, const char* dir)
{
    if (!path || !dir)
        return NULL;

    strcpy(path, getenv("HOME"));
    strcat(path, dir);
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

static char *load_file(const char *file)
{
    char path[PATH_MAX];
    char *readstring = NULL;
    size_t reclen, bufferlen;
    struct stat st;
    int fd;

    assert(file);
    assert(*file);

    reclen = snprintf(path, PATH_MAX, "../etc/did/resources/testdata/%s", file);
        if (reclen < 0 || reclen > PATH_MAX)
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

bool file_exist(const char *path)
{
    return test_path(path) == S_IFREG;
}

bool dir_exist(const char* path)
{
    return test_path(path) == S_IFDIR;
}

/////////////////////////////////////
static int import_privatekey(DIDURL *id, const char *file)
{
    char *skbase;
    DIDStore *store;
    uint8_t privatekey[PRIVATEKEY_BYTES];
    char privatekeybase64[MAX_PRIVATEKEY_BASE64];

    if (!id || !file || !*file)
        return -1;

    skbase = load_file(file);
    if (!skbase || !*skbase)
        return -1;

    if (base58_decode(privatekey, skbase) != PRIVATEKEY_BYTES) {
        free(skbase);
        return -1;
    }

    store = DIDStore_GetInstance();
    if (encrypt_to_base64((char *)privatekeybase64, storepass, privatekey, sizeof(privatekey)) == -1 ||
        DIDStore_StorePrivateKey(store, DIDURL_GetDid(id), id, (const char *)privatekeybase64) == -1) {
        free(skbase);
        return -1;
    }

    return 0;
}

DIDStore *TestData_SetupStore(const char *root)
{
    char _dir[PATH_MAX],_path[PATH_MAX];
    char *walletDir, *storePath;

    if (!root || !*root)
        return NULL;

    if (!testdata.adapter) {
        walletDir = get_wallet_path(_dir, walletdir);
        testdata.adapter = TestDIDAdapter_Create(walletDir, walletId, network, resolver, getpassword);
    }

    delete_file(root);
    testdata.store = DIDStore_Initialize(root, testdata.adapter);
    return testdata.store;
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

const char *TestData_LoadVcProfileJson(void)
{
    if (!testdata.profileVcJson)
        testdata.profileVcJson = load_file("vc-profile.json");

    return testdata.profileVcJson;
}

const char *TestData_LoadVcProfileCompJson(void)
{
    if (!testdata.profileVcCompactJson)
        testdata.profileVcCompactJson = load_file("vc-profile.compact.json");

    return testdata.profileVcCompactJson;
}

const char *TestData_LoadVcProfileNormJson(void)
{
    if (!testdata.profileVcNormalizedJson)
        testdata.profileVcNormalizedJson = load_file("vc-profile.normalized.json");

    return testdata.profileVcNormalizedJson;
}

const char *TestData_LoadVcEmailJson(void)
{
    if (!testdata.emailVcJson)
        testdata.emailVcJson = load_file("vc-email.json");

    return testdata.emailVcJson;
}

const char *TestData_LoadVcEmailCompJson(void)
{
    if (!testdata.emailVcCompactJson)
        testdata.emailVcCompactJson = load_file("vc-email.compact.json");

    return testdata.emailVcCompactJson;
}

const char *TestData_LoadVcEmailNormJson(void)
{
    if (!testdata.emailVcNormalizedJson)
        testdata.emailVcNormalizedJson = load_file("vc-email.normalized.json");

    return testdata.emailVcNormalizedJson;
}

const char *TestData_LoadVcPassportJson(void)
{
    if (!testdata.passportVcJson)
        testdata.passportVcJson = load_file("vc-passport.json");

    return testdata.passportVcJson;
}

const char *TestData_LoadVcPassportCompJson(void)
{
    if (!testdata.passportVcCompactJson)
        testdata.passportVcCompactJson = load_file("vc-passport.compact.json");

    return testdata.passportVcCompactJson;
}

const char *TestData_LoadVcPassportNormJson(void)
{
    if (!testdata.passportVcNormalizedJson)
        testdata.passportVcNormalizedJson = load_file("vc-passport.normalized.json");

    return testdata.passportVcNormalizedJson;
}

const char *TestData_LoadVcTwitterJson(void)
{
    if (!testdata.twitterVcJson)
        testdata.twitterVcJson = load_file("vc-twitter.json");

    return testdata.twitterVcJson;
}

const char *TestData_LoadVcTwitterCompJson(void)
{
    if (!testdata.twitterVcCompactJson)
        testdata.twitterVcCompactJson = load_file("vc-twitter.compact.json");

    return testdata.twitterVcCompactJson;
}

const char *TestData_LoadVcTwitterNormJson(void)
{
    if (!testdata.twitterVcNormalizedJson)
        testdata.twitterVcNormalizedJson = load_file("vc-twitter.normalized.json");

    return testdata.twitterVcNormalizedJson;
}

const char *TestData_LoadVpJson(void)
{
    if (!testdata.vpJson)
        testdata.vpJson = load_file("vp.json");

    return testdata.vpJson;
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

    const char *docstring = TestData_LoadDocJson();
    if (!docstring || !*docstring)
        return NULL;

    testdata.doc = DIDDocument_FromJson(docstring);
    if (!testdata.doc)
        return NULL;
    subject = DIDDocument_GetSubject(testdata.doc);

    id = DIDURL_FromDid(subject, "key2");
    rc = import_privatekey(id, "doc.key2.sk");
    DIDURL_Destroy(id);
    if (rc)
        return NULL;

    id = DIDURL_FromDid(subject, "key3");
    rc = import_privatekey(id, "doc.key3.sk");
    DIDURL_Destroy(id);
    if (rc)
        return NULL;

    id = DIDURL_FromDid(subject, "primary");
    rc = import_privatekey(id, "doc.primary.sk");
    DIDURL_Destroy(id);
    if (rc)
        return NULL;

    return testdata.doc;
}

DIDDocument *TestData_LoadIssuerDoc(void)
{
    DIDURL *id;
    DID *subject;
    int rc;

    const char *docstring = TestData_LoadIssuerJson();
    if (!docstring || !*docstring)
        return NULL;

    testdata.issuerdoc = DIDDocument_FromJson(docstring);
    if (!testdata.issuerdoc)
        return NULL;
    subject = DIDDocument_GetSubject(testdata.issuerdoc);

    id = DIDURL_FromDid(subject, "primary");
    rc = import_privatekey(id, "issuer.primary.sk");
    DIDURL_Destroy(id);
    if (rc)
        return NULL;

    return testdata.issuerdoc;
}

void TestData_Free(void)
{
    DIDStore_Deinitialize();

    if (testdata.adapter)
        TestDIDAdapter_Destroy(testdata.adapter);

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

    if (testdata.profileVcJson)
        free(testdata.profileVcJson);
    if (testdata.profileVcCompactJson)
        free(testdata.profileVcCompactJson);
    if (testdata.profileVcNormalizedJson)
        free(testdata.profileVcNormalizedJson);

    if (testdata.emailVcJson)
        free(testdata.emailVcJson);
    if (testdata.emailVcCompactJson)
        free(testdata.emailVcCompactJson);
    if (testdata.emailVcNormalizedJson)
        free(testdata.emailVcNormalizedJson);

    if (testdata.passportVcJson)
        free(testdata.passportVcJson);
    if (testdata.passportVcCompactJson)
        free(testdata.passportVcCompactJson);
    if (testdata.passportVcNormalizedJson)
        free(testdata.passportVcNormalizedJson);

    if (testdata.twitterVcJson)
        free(testdata.twitterVcJson);
    if (testdata.twitterVcCompactJson)
        free(testdata.twitterVcCompactJson);
    if (testdata.twitterVcNormalizedJson)
        free(testdata.twitterVcNormalizedJson);

    if (testdata.vpJson)
        free(testdata.vpJson);
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
    uint8_t seed[SEED_BYTES];
    uint8_t publickey[PUBLICKEY_BYTES];
    MasterPublicKey _mk, *masterkey;

    if (size < MAX_PUBLICKEY_BASE58)
        return NULL;

    mnemonic = Mnemonic_Generate(0);
    if (!mnemonic || !*mnemonic)
        return NULL;

    if (!HDkey_GetSeedFromMnemonic(mnemonic, "", 0, seed)) {
        Mnemonic_free((char*)mnemonic);
        return NULL;
    }

    masterkey = HDkey_GetMasterPublicKey(seed, 0, &_mk);
    if (!masterkey) {
        Mnemonic_free((char*)mnemonic);
        return NULL;
    }

    if (!HDkey_GetSubPublicKey(masterkey, 0, 0, publickey)) {
        Mnemonic_free((char*)mnemonic);
        return NULL;
    }

    base58_encode(publickeybase58, publickey, sizeof(publickey));
    return publickeybase58;
}