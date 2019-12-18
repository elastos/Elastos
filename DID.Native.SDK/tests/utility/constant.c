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

const char *walletId = "cywallet";
const char *network = "../../../../adapter/wallet/privnet.json";
const char *resolver = "https://coreservices-didsidechain-privnet.elastos.org";
const char *storepass = "12345678";

const char *type = "ECDSAsecp256r1";
const char *service_type = "CarrierAddress";

const char *mnemonic = "cloth always junk crash fun exist stumble shift over benefit fun toe";

const char *testdid_string = "did:elastos:icwTktC5M6fzySQ5yU7bKAZ6ipP623apFY";
const char *testid_string = "did:elastos:icwTktC5M6fzySQ5yU7bKAZ6ipP623apFY#default";
const char *method_specific_string = "icwTktC5M6fzySQ5yU7bKAZ6ipP623apFY";
const char *fragment = "default";
const char *compact_idstring = "#default";

const char *storetag = "/.DIDStore";
const char *docstring = "/document";