#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>
#include <signal.h>
#include <unistd.h>

#include "ela_did.h"

static char *MAINTYPE = "MainNet";
static char *TESTTYPE = "TestNet";
static char *MAINNET = "http://api.elastos.io:20606";
static char *TESTNET = "http://api.elastos.io:21606";

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>

static int sys_coredump_set(bool enable) {
    const struct rlimit rlim = {
        enable ? RLIM_INFINITY : 0,
        enable ? RLIM_INFINITY : 0
    };

    return setrlimit(RLIMIT_CORE, &rlim);
}
#endif

static void usage(void)
{
    fprintf(stdout, "DID Resolver\n");
    fprintf(stdout, "Usage agent [OPTION]\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "  -n, --network=type           The net type to resolve DID.\n");
    fprintf(stdout, "  -d, --did=DID                DID string\n");
    fprintf(stdout, "\n");
}

int main(int argc, char *argv[])
{
    char *url = MAINNET, *nettype = MAINTYPE, *idstring;
    char cachedir[PATH_MAX];
    const char *data;
    DID *did;
    DIDDocument *doc;

    int opt;
    int idx;
    struct option options[] = {
        { "network",        optional_argument,   NULL, 'n' },
        { "did",            required_argument,   NULL, 'd' },
        { "help",           no_argument,         NULL, 'h' },
        { NULL,             0,                   NULL,  0  }
    };

#ifdef HAVE_SYS_RESOURCE_H
    sys_coredump_set(true);
#endif

    while ((opt = getopt_long(argc, argv, "n:d:h?", options, &idx)) != -1) {
        switch (opt) {
        case 'n':
            nettype = optarg;
            break;

        case 'd':
            idstring = optarg;
            break;

        case 'h':
        case '?':
        default:
            usage();
            exit(-1);
        }
    }

    if (!strcmp(nettype, TESTTYPE))
        url = TESTNET;

    sprintf(cachedir, "%s%s", getenv("HOME"), "/.cache.did.elastos");
    if (DIDBackend_InitializeDefault(url, cachedir) < 0) {
        printf("Initial resolver failed. Error: %s\n", DIDError_GetMessage());
        goto cleanup;
    }

    did = DID_New(idstring);
    if (!did) {
        printf("Create DID failed. Error: %s\n", DIDError_GetMessage());
        goto cleanup;
    }

    doc = DID_Resolve(did, true);
    DID_Destroy(did);
    if (!doc) {
        printf("Resolve [%s] failed. Error: %s\n", idstring, DIDError_GetMessage());
        goto cleanup;
    }

    data = DIDDocument_ToString(doc, true);
    DIDDocument_Destroy(doc);
    if (!data) {
        printf("Invalid document.\n");
        goto cleanup;
    }

    printf("Document resolved:\n%s\n\n", data);
    free((void*)data);

cleanup:
    return 0;
}

