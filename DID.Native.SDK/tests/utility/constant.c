#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <limits.h>
#include <crystal.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <fnmatch.h>

#include "constant.h"

const char *walletdir = ".elawallet";
const char *walletId = "cywallet";
//const char *network = "../../../../adapter/wallet/privnet.json";
const char *network = "TestNet";
//const char *resolver = "https://coreservices-didsidechain-privnet.elastos.org";
const char *resolver = "http://api.elastos.io:21606";
const char *walletpass = "12345678";

const char *storepass = "123456";
const char *passphase = "";
const char *default_type = "ECDSAsecp256r1";
const char *service_type = "CarrierAddress";

const char *mnemonic = "cloth always junk crash fun exist stumble shift over benefit fun toe";
const char *language = "english";

const char *testdid_string = "did:elastos:iWFAUYhTa35c1fPe3iCJvihZHx6quumnym";
const char *testid_string = "did:elastos:iWFAUYhTa35c1fPe3iCJvihZHx6quumnym#default";
const char *method_specific_string = "iWFAUYhTa35c1fPe3iCJvihZHx6quumnym";
const char *fragment = "default";
const char *compact_idstring = "#default";

const char *PATH_STEP = "/";
const char *PRIVATE_DIR = "private";
const char *HDKEY_FILE = "key";
const char *HDPUBKEY_FILE = "key.pub";
const char *INDEX_FILE = "index";
const char *MNEMONIC_FILE = "mnemonic";

const char *DID_DIR = "ids";
const char *DOCUMENT_FILE = "document";
const char *CREDENTIALS_DIR = "credentials";
const char *CREDENTIAL_FILE = "credential";
const char *PRIVATEKEYS_DIR = "privatekeys";
const char *META_FILE = ".meta";