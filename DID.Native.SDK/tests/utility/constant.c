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

const char *walletdir = "/.wallet.testnet";
const char *walletId = "cywallet";
//const char *network = "../../../../adapter/wallet/privnet.json";
const char *network = "TestNet";
//const char *resolver = "https://coreservices-didsidechain-privnet.elastos.org";
const char *resolver = "http://api.elastos.io:21606";
const char *walletpass = "12345678";

const char *storepass = "123456";
const char *type = "ECDSAsecp256r1";
const char *service_type = "CarrierAddress";

const char *mnemonic = "cloth always junk crash fun exist stumble shift over benefit fun toe";

const char *testdid_string = "did:elastos:iYpQMwheDxySqivocSJaoprcoDTqQsDYAu";
const char *testid_string = "did:elastos:iYpQMwheDxySqivocSJaoprcoDTqQsDYAu#default";
const char *method_specific_string = "iYpQMwheDxySqivocSJaoprcoDTqQsDYAu";
const char *fragment = "default";
const char *compact_idstring = "#default";

const char *storetag = "/.meta";
const char *docstring = "/document";

const char *privateindex = "/private/index";
const char *privatekey = "/private/key";
const char *privatemnemonic = "/private/mnemonic";
const char *storedirroot = "/ids";
const char *metastring = "/.meta";

const char *teststore_path = "../etc/did/resources/teststore";