#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <getopt.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

#include "ela_did.h"

#define METHOD "did:elastos:"
#define METHODLEN strlen(METHOD)

struct AgentCtx {
    DIDStore *store;
    char storepass[256];
    char *lang;
};

struct AgentCtx ctx = {
    NULL,
    {0},
    "english"
};

static void clean(char *input)
{
    char *p = input;

    while (*p != '\0') {
        if (*p == '\n')
            *p = '\0';
        else
            p++;
    }
}

static void passwd(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    printf("Sorry :(, not supported yet.\n");
}

static void newdid(int argc, char *argv[])
{
    DIDDocument *doc;
    const char *didstr;

    if (argc != 2 || !*argv[1]) {
        printf("Error: invalid command syntax\n");
        return;
    }

    doc = DIDStore_NewDID(ctx.store, ctx.storepass, argv[1]);
    if (!doc) {
        printf("Error: new did document failed.\n");
        return;
    }

    didstr = DID_GetMethodSpecificId(DIDDocument_GetSubject(doc));
    printf("Grats! DID(%s) with alias(%s) created.\n", didstr, argv[1]);

    DIDDocument_Destroy(doc);
}

static int iterate_did_cb(DID *did, void *context)
{
    int *num = (int *)context;

    if (!did)
        return 1;

    printf("  %s\t%s\n", DID_GetMethodSpecificId(did), DID_GetAlias(did));

    *num += 1;
    return 0;
}

static void list_dids(int argc, char *argv[])
{
    int rc;
    int num = 0;

    if (argc != 1) {
        printf("Error: invalid command syntax\n");
        return;
    }

    printf("DIDs listed:\n  did \t alias\n");
    rc = DIDStore_ListDIDs(ctx.store, DID_FILTER_ALL, iterate_did_cb, &num);
    if (rc < 0) {
        printf("Error: list DIDs failed.\n");
        return;
    }

    if (num <= 0) {
        printf("  <N/A>\n");
    }
    printf("\n");
}

static DID *from_didstr(const char *idstr)
{
    return strncmp(idstr, METHOD, METHODLEN) ?
                DID_New(idstr) : DID_FromString(idstr);
}

static void rmdid(int argc, char *argv[])
{
    DID *did;
    bool yes;

    if (argc != 2) {
        printf("Error: invalid command syntax\n");
        return;
    }

    did = from_didstr(argv[1]);
    if (!did) {
        printf("Error: invalid did string: %s\n", argv[1]);
        return;
    }

    yes = DIDStore_ContainsDID(ctx.store, did);
    if (!yes) {
        DID_Destroy(did);
        printf("No did(%s) exists\n", argv[1]);
        return;
    }

    yes = DIDStore_DeleteDID(ctx.store, did);
    DID_Destroy(did);

    if (!yes) {
        printf("Error: remove did(%s) failed\n", argv[1]);
        return;
    }

    printf("Grats! DID(%s) removed\n", argv[1]);
}

static void edit_doc(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    printf("Comming soon :)\n");
}

#define SIGNATURE_BYTES         64

static void sign(int argc, char *argv[])
{
    DID *did;
    DIDURL *didurl;
    DIDDocument *doc;
    int rc;
    char signature[SIGNATURE_BYTES * 2 + 16] = {0};

    if (argc != 3) {
        printf("Error: invalid command syntax\n");
        return;
    }

    if (strlen(argv[2]) >= 32) {
        printf("Error: data too long (< 32)\n");
        return;
    }

    did = from_didstr(argv[1]);
    if (!did) {
        printf("Error: invalid did string: %s\n", argv[1]);
        return;
    }

    doc = DIDStore_LoadDID(ctx.store, did);
    DID_Destroy(did);

    if (!doc) {
        printf("Error: no such DID %s exists.\n", argv[1]);
        return;
    }

    did = DIDDocument_GetSubject(doc);
    didurl = DIDDocument_GetDefaultPublicKey(doc);

    rc = DIDDocument_Sign(doc, didurl, ctx.storepass, signature, 1, argv[2], strlen(argv[2]));
    DIDDocument_Destroy(doc);

    if (rc < 0) {
        printf("Error: sign on data(%s) failed.\n", argv[2]);
        return;
    }

    printf("Grats! signed data(%s) with did(%s)\n", argv[2], argv[1]);
    printf("Got signature: %s\n", signature);
}

static void verify(int argc, char *argv[])
{
    DID *did;
    DIDURL *didurl;
    DIDDocument *doc;
    char sign[SIGNATURE_BYTES * 2 + 16] = {0};
    int rc;

    if (argc != 4) {
        printf("Error: invaid command syntax\n");
        return;
    }

    if (strlen(argv[2]) >= sizeof(sign)) {
        printf("Error: signature too long (<%lu)\n", sizeof(sign));
        return;
    }

    memcpy(sign, argv[2], strlen(argv[2]));

    if (strlen(argv[3]) >= 32) {
        printf("Error: data too long (<32).\n");
        return;
    }

    did = from_didstr(argv[1]);
    if (!did) {
        printf("Error: invalid did string: %s\n", argv[1]);
        return;
    }

    doc = DIDStore_LoadDID(ctx.store, did);
    DID_Destroy(did);

    if (!doc) {
        printf("Error: no such DID %s exists.\n", argv[1]);
        return;
    }

    didurl = DIDDocument_GetDefaultPublicKey(doc);

    rc = DIDDocument_Verify(doc, didurl, sign, 1, argv[3], strlen(argv[3]));
    DIDDocument_Destroy(doc);

    if (rc < 0) {
        printf("Error: verify signature(%s) on data(%s) failed\n", argv[2], argv[3]);
        return;
    }

    printf("Grats! Verification on data(%s) with signature(%s) passed\n", argv[3], argv[2]);
}

static void clear_screen(int argc, char *argv[])
{
    (void)ctx;

    if (argc != 1) {
        printf("Error: invalid command syntax\n");
        return;
    }

    system(argv[0]);
}

static void help(int argc, char *argv[]);

struct command {
    const char *cmd;
    void (*func)(int argc, char *argv[]);
    const char *args;
    const char *help;
} commands[] = {
    { "passwd",     passwd,             "old new",  "Change storepass" },
    { "new",        newdid,             "alias",    "Create a new did" },
    { "list",       list_dids,          NULL,       "List all dids"},
    { "remove",     rmdid,             "did",      "Remove specific did"},
    { "edit",       edit_doc,           "did",      "Edit DID document"},
    { "sign",       sign,               "did data", "Sign data with specific DID" },
    { "verify",     verify,             "did signature data", "Verify signature on data" },
    { "clear",      clear_screen,       NULL,       "Clear the command screen"},
    { "help",       help,               NULL,       "Show all commands" },
    { "exit",       NULL,               NULL,       "Quit from agent" },
    { "quit",       NULL,               NULL,       "quit from agent" },
    { NULL,         NULL,               NULL,       NULL }
};

static void help(int argc, char *argv[])
{
    struct command *p;

    if (argc == 1) {
        fprintf(stdout, "Available commands listed:\n");

        for (p = commands; p->cmd; p++) {
            if (strlen(p->cmd) >= 6)
                printf("  %s\t%s\n", p->cmd, p->help);
            else
                printf("  %s\t\t%s\n", p->cmd, p->help);
        }
        fprintf(stdout, "\n");
   } else {
        for (p = commands; p->cmd; p++) {
            if (strcmp(argv[1], p->cmd) == 0) {
                fprintf(stdout, "Syntax: %s %s\n\n", p->cmd, p->args ? p->args: "");
                return;
            }
        }

        fprintf(stderr, "Unknown command: %s\n", argv[1]);
    }
}

int run_cmd(int argc, char *argv[])
{
    struct command *p;

    for (p = commands; p->cmd; p++) {
        if (strcmp(argv[0], p->cmd) == 0) {
            p->func(argc, argv);
            return 0;
        }
    }

    fprintf(stderr, "Unknown command: %s\n", argv[0]);
    return -1;
}

static int parse_cmd(char *cmdLine, char *argv[])
{
    char *p;
    int arg = 0;
    int count = 0;

    for (p = cmdLine; *p != 0; p++) {
        if (isspace(*p)) {
            *p = 0;
            arg = 0;
        } else {
            if (arg == 0) {
                argv[count] = p;
                count++;
            }

            arg = 1;
        }
    }

    return count;
}

static int mkdir_internal(const char *path, mode_t mode) {
    struct stat st;
    int rc = 0;

    if (stat(path, &st) != 0) {
        /* Directory does not exist. EEXIST for race condition */
        if (mkdir(path, mode) != 0 && errno != EEXIST)
            rc = -1;
    } else if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        rc = -1;
    }

    return rc;
}

static int mkdirs(const char *path, mode_t mode)
{
    int rc = 0;
    char *pp;
    char *sp;
    char copypath[PATH_MAX];

    strncpy(copypath, path, sizeof(copypath));
    copypath[sizeof(copypath) - 1] = 0;

    pp = copypath;
    while (rc == 0 && (sp = strchr(pp, '/')) != 0) {
        if (sp != pp) {
            /* Neither root nor double slash in path */
            *sp = '\0';
            rc = mkdir_internal(copypath, mode);
            *sp = '/';
        }
        pp = sp + 1;
    }

    if (rc == 0)
        rc = mkdir_internal(path, mode);

    return rc;
}

static char *get_datadir(const char *dir)
{
    static char path[PATH_MAX];

    if (!dir) {
        sprintf(path, "%s/.didstore", getenv("HOME"));
    }

    return path;
}

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
    fprintf(stdout, "  -d, --data=PATH              DIDStore directory\n");
    fprintf(stdout, "      --debug                  Wait for debugger to attach\n");
    fprintf(stdout, "\n");
}

const char *create_id_transaction(DIDAdapter *adapter,
                                  const char *payload, const char *memo)
{
    // TODO:
    return NULL;
}

static int gen_priv_identity(struct AgentCtx *ctx)
{
    char passphrase[64] = {0};
    const char *mnemonic;
    int rc;

    mnemonic = Mnemonic_Generate(ctx->lang);
    if (!mnemonic) {
        printf("Error: generate mnemonic failed.\n");
        return -1;
    }

    printf(" Input your storepass:\n");
    fgets(ctx->storepass, sizeof(ctx->storepass), stdin);
    clean(ctx->storepass);

    printf(" Input your phrasepass:\n");
    fgets(passphrase, sizeof(passphrase), stdin);
    clean(passphrase);

    rc = DIDStore_InitPrivateIdentity(ctx->store, ctx->storepass, mnemonic, passphrase, ctx->lang, false);
    if (rc < 0) {
        printf("Error: initialize private identity failed.\n");
        return -1;
    }

    printf("Grats! Private identiy generated in this DID store.\n");
    return 0;
}

int main(int argc, char *argv[])
{
    int wait_for_attach = 0;
    char *datadir = NULL;

    DIDAdapter adapter;
    DIDStore *store;
    int rc;

    int opt;
    int idx;
    struct option options[] = {
        { "data",           required_argument,  NULL, 'd' },
        { "debug",          no_argument,        NULL,  1  },
        { "help",           no_argument,        NULL, 'h' },
        { NULL,             0,                  NULL,  0  }
    };

#ifdef HAVE_SYS_RESOURCE_H
    sys_coredump_set(true);
#endif

    while ((opt = getopt_long(argc, argv, "d:h?", options, &idx)) != -1) {
        switch (opt) {
        case 'd':
            datadir = optarg;
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

    datadir = get_datadir(datadir);

    rc = mkdirs(datadir, S_IRWXU);
    if (rc < 0) {
        printf("Error: create DID store home directory at '%s' failed.\n", datadir);
        return -1;
    }

    printf("DID store home directory at '%s'\n", datadir);


    adapter.createIdTransaction = create_id_transaction;
    store = DIDStore_Open(datadir, &adapter);
    if (!store) {
        printf("Error: open DID store at '%s' failed.\n", datadir );
        return -1;
    }

    memset(&ctx, 0, sizeof(ctx));
    ctx.store = store;
    ctx.lang  = "english";

    if (!DIDStore_ContainsPrivateIdentity(store)) {
        char input[32] = {0};
        int which;

        printf("Oops, empty DID store. Have to choose one of the ways listed below:\n"
               " 1) Initialize from scratch;\n"
               " 2) Import mnemonic\n"
               " 3) Import root key\n"
               "to carry out the initialization of DID store.\n");

        fgets(input, sizeof(input), stdin);
        sscanf(input, "%d\n", &which);

        if (which <= 0 || which > 3) {
            printf("Error: invalid way. Please choose 1/2/3.\n");
            return -1;
        }

        switch (which) {
        case 1:
            rc = gen_priv_identity(&ctx);
            if (rc < 0) {
                printf("Error: generate private identity failed\n");
                return -1;
            }

            break;

        case 2:
        case 3:
        default:
            printf("Not supported yet, but comming soon\n");
            return 0;
        }
    } else {
        printf("Need to input your storepass: \n");
        fgets(ctx.storepass, sizeof(ctx.storepass), stdin);
        clean(ctx.storepass);
    }

    while (1) {
        char buff[4096];
        char *argv[256];
        int argc;

        fprintf(stdout, "$ ");
        fgets(buff, sizeof(buff), stdin);

        argc = parse_cmd(buff, argv);
        if (argc == 0)
            continue;

        if (strcmp(argv[0], "quit") == 0 ||
            strcmp(argv[0], "exit") == 0)
            break;

        run_cmd(argc, argv);
    }

    DIDStore_Close(store);
}

