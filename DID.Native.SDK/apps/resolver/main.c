#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>
#include <signal.h>
#include <unistd.h>

#include "ela_did.h"

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

void signal_handler(int signum)
{
    exit(-1);
}

static void usage(void)
{
    fprintf(stdout, "DID CLI agent\n");
    fprintf(stdout, "Usage agent [OPTION]\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "  -u, --url=URL                The url for resolver\n");
    fprintf(stdout, "  -d, --did=DID                DID string\n");
    fprintf(stdout, "      --debug                  Wait for debugger to attach\n");
    fprintf(stdout, "\n");
}

static char *get_input(char *input, size_t size)
{
    int len;

    fgets(input, size, stdin);
    len = strlen(input);
    input[len - 1] = 0;

    if (!strcmp(input, "reset"))
        return NULL;

    return input;
}

int main(int argc, char *argv[])
{
    int wait_for_attach = 0;
    char url[1024] = {0};
    char idstring[ELA_MAX_DID_LEN] = {0};
    char cachedir[PATH_MAX];
    int rc, url_index = 0, did_index = 0;
    DID *did = NULL;
    DIDDocument *doc = NULL;

    int opt;
    int idx;
    struct option options[] = {
        { "url",            optional_argument,   NULL, 'u' },
        { "did",            optional_argument,   NULL, 'd' },
        { "debug",          no_argument,         NULL,  1  },
        { "help",           no_argument,         NULL, 'h' },
        { NULL,             0,                   NULL,  0  }
    };

#ifdef HAVE_SYS_RESOURCE_H
    sys_coredump_set(true);
#endif

    while ((opt = getopt_long(argc, argv, "u:d:h?", options, &idx)) != -1) {
        switch (opt) {
        case 'u':
            strncpy(url, optarg, sizeof(url));
            break;

        case 'd':
            strncpy(idstring, optarg, sizeof(idstring));
            break;

        case 1:
            wait_for_attach = 1;
            break;

        case 'h':
        case '?':
        default:
            usage();
            exit(-1);
        }
    }

    if (wait_for_attach) {
        printf("Wait for debugger attaching, process id is: %d.\n", getpid());
#ifndef _MSC_VER
        printf("After debugger attached, press any key to continue......");
        getchar();
#else
        DebugBreak();
#endif
    }

    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGKILL, signal_handler);
    signal(SIGHUP,  signal_handler);

    printf("Welcome to resolver! You can input 'reset' to reset all data as follow.\n");

    while (1) {
        url_index++;

        if (!*url) {
           printf("Please input resolver url: \n");
           if (!get_input(url, sizeof(url)))
               goto cleanup;
        }

        sprintf(cachedir, "%s%s", getenv("HOME"), "/.cache.did.elastos");
        if (DIDBackend_InitializeDefault(url, cachedir) < 0) {
            printf("Initial resolver failed. Error: %s\n", DIDError_GetMessage());
            goto cleanup;
        }

        url_index = 0;
        do {
            memset(idstring, 0, sizeof(idstring));
            printf("Please input did string: \n");
            if (!get_input(idstring, sizeof(idstring)))
                goto cleanup;

            did = DID_FromString(idstring);
            if (!did)
                printf("Create DID failed. Error: %s\n", DIDError_GetMessage());
        } while (!did && did_index++ <= 10);

        if (!did)
            goto cleanup;

        doc = DID_Resolve(did, true);
        DID_Destroy(did);
        if (!doc) {
            printf("Resolve [%s] failed. Error: %s\n", idstring, DIDError_GetMessage());
            goto cleanup;
        }

        const char *data = DIDDocument_ToString(doc, true);
        if (!data) {
            printf("Invalid document.\n");
            goto cleanup;
        }

        printf("Document resolved:\n%s\n\n", data);
        printf("-----------------------\nBegin new resolver!\n");

cleanup:
        memset(url, 0, sizeof(url));
        memset(idstring, 0, sizeof(idstring));
        DIDDocument_Destroy(doc);
        if (data)
            free((void*)data);

        if (url_index >= 10) {
            printf("More wrong inputs. Please press any key to exit resolver.\n");
            getchar();
            exit(-1);
        }
    }
}

