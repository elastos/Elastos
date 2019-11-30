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

#include "constant.h"

const char *walletId = "chenyuwallet";
const char *network = "../../../../adapter/wallet/privnet.json";
const char *resolver = "https://coreservices-didsidechain-privnet.elastos.org";
const char *storepass = "12345678";

const char *type = "ECDSAsecp256r1";
const char *service_type = "OpenIdConnectVersion1.0Service";

const char *mnemonic = "cloth always junk crash fun exist stumble shift over benefit fun toe";

char *get_store_path(char* path, const char *dir)
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

char *get_wallet_path(char* path, const char* dir)
{
    if (!path || !dir)
        return NULL;

    strcpy(path, getenv("HOME"));
    strcat(path, dir);
    return path;
}