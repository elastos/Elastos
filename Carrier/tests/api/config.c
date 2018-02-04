#include <stdlib.h>
#include <string.h>
#include <confuse.h>

#include "rc_mem.h"
#include "config.h"

TestConfig global_config;

static void config_error(cfg_t *cfg, const char *fmt, va_list ap)
{
    fprintf(stderr, "Config file error, line %d: ", cfg->line);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
}

static int get_str(cfg_t *cfg, const char *name, char *value, size_t len)
{
    char *stropt;
    
    stropt = cfg_getstr(cfg, name);
    if (!stropt) {
        cfg_error(cfg, "Option %s missing.", name);
        cfg_free(cfg);
        exit(-1);
    }

    if (strlen(stropt) >= len) {
        cfg_error(cfg, "Option %s too long.", name);
        cfg_free(cfg);
        exit(-1);
    }

    strcpy(value, stropt);
    return 0;
}

static int bootstrap_validator(cfg_t *cfg, cfg_opt_t *opt)
{
    cfg_t *sec;

    sec = cfg_opt_getnsec(opt, cfg_opt_size(opt) - 1);

    if (cfg_getstr(sec, "ipv4") == NULL &&
        cfg_getstr(sec, "ipv6") == NULL) {
        cfg_error(cfg, "ipv4 and ipv6 options both missing.");
        return -1;
    }

    if (cfg_getstr(sec, "port") == NULL) {
        cfg_error(cfg, "port option missing.");
        return -1;
    }

    if (cfg_getstr(sec, "public_key") == NULL) {
        cfg_error(cfg, "public_key option missing");
        return -1;
    }

    return 0;
}

static int bootstraps_validator(cfg_t *cfg, cfg_opt_t *opt)
{
    int nbootstraps;

    nbootstraps = cfg_size(cfg, "bootstraps|bootstrap");
    if (nbootstraps == 0) {
        cfg_error(cfg, "no bootstraps defined.");
        return -1;
    }

    return 0;
}

static void bootstrap_destroy(void *p)
{
    BootstrapNode *node = (BootstrapNode *)p;

    if (!node)
        return;

    if (node->ipv4)
        free((void *)node->ipv4);

    if (node->ipv6)
        free((void *)node->ipv6);

    if (node->port)
        free((void *)node->port);

    if (node->public_key)
        free((void *)node->public_key);
}

static void free_bootstraps(TestConfig *config)
{
    int i = 0;

    if (!config->bootstraps)
        return;

    for (i = 0; i < config->bootstraps_size; i++) {
        deref(&config->bootstraps[i]);
        free(config->bootstraps[i]);
    }

    free(config->bootstraps);\
}

void load_config(const char *config_file)
{
    cfg_t *cfg, *bootstraps, *sec;
    char *stropt;
    int nsecs;
    int i;
    int rc;

    cfg_opt_t bootstrap_opts[] = {
        CFG_STR("ipv4", NULL, CFGF_NONE),
        CFG_STR("ipv6", NULL, CFGF_NONE),
        CFG_STR("port", "33445", CFGF_NONE),
        CFG_STR("public_key", NULL, CFGF_NONE),
        CFG_END()
    };

    cfg_opt_t bootstraps_opts[] = {
        CFG_SEC("bootstrap", bootstrap_opts, CFGF_MULTI | CFGF_NO_TITLE_DUPES),
    };

    cfg_opt_t cfg_opts[] = {
        CFG_INT("shuffle", 1, CFGF_NONE),
        CFG_INT("log2file", 0, CFGF_NONE),
        CFG_INT("tests.loglevel", 4, CFGF_NONE),
        CFG_STR("tests.data_location", ".tests", CFGF_NONE),
        CFG_INT("robot.loglevel", 4, CFGF_NONE),
        CFG_STR("robot.data_location", ".robot", CFGF_NONE),
        CFG_SEC("bootstraps", bootstraps_opts, CFGF_NONE),
        CFG_END()
    };

    cfg = cfg_init(cfg_opts, CFGF_NONE);
    cfg_set_error_function(cfg, config_error);
    cfg_set_validate_func(cfg, "bootstraps", bootstraps_validator);
    cfg_set_validate_func(cfg, "bootstraps|bootstrap", bootstrap_validator);

    rc = cfg_parse(cfg, config_file);
    if (rc != CFG_SUCCESS) {
        cfg_error(cfg, "can not pase config file: %s.", config_file);
        cfg_free(cfg);
        exit(-1);
    }

    global_config.shuffle = (int)cfg_getint(cfg, "shuffle");
    global_config.log2file = (int)cfg_getint(cfg, "log2file");

    get_str(cfg, "tests.data_location", global_config.tests.data_location, sizeof(global_config.tests.data_location));
    global_config.tests.loglevel = (int)cfg_getint(cfg, "tests.loglevel");

    get_str(cfg, "robot.data_location", global_config.robot.data_location, sizeof(global_config.robot.data_location));
    global_config.robot.loglevel = (int)cfg_getint(cfg, "robot.loglevel");

    bootstraps = cfg_getsec(cfg, "bootstraps");
    if (!bootstraps) {
        cfg_error(cfg, "missing services section.");
        cfg_free(cfg);
        exit(-1);
    }

    nsecs = cfg_size(bootstraps, "bootstrap");

    global_config.bootstraps_size = nsecs;
    global_config.bootstraps = (BootstrapNode **)calloc(1, global_config.bootstraps_size *
                                                  sizeof(BootstrapNode *));
    if (!global_config.bootstraps) {
        cfg_error(cfg, "out of memory.");
        cfg_free(cfg);
        free_bootstraps(&global_config);
        exit(-1);
    }

    for (i = 0; i < nsecs; i++) {
        BootstrapNode *node;

        node = rc_zalloc(sizeof(BootstrapNode), bootstrap_destroy);
        if (!node) {
            cfg_error(cfg, "out of memory.");
            cfg_free(cfg);
            free_bootstraps(&global_config);
            exit(-1);
        }

        sec = cfg_getnsec(bootstraps, "bootstrap", i);

        stropt = cfg_getstr(sec, "ipv4");
        if (stropt)
            node->ipv4 = (const char *)strdup(stropt);
        else
            node->ipv4 = NULL;

        stropt = cfg_getstr(sec, "ipv6");
        if (stropt)
            node->ipv6 = (const char *)strdup(stropt);
        else
            node->ipv6 = NULL;

        stropt = cfg_getstr(sec, "port");
        if (stropt)
            node->port = (const char *)strdup(stropt);
        else
            node->port = NULL;

        stropt = cfg_getstr(sec, "public_key");
        if (stropt)
            node->public_key = (const char *)strdup(stropt);
        else
            node->public_key = NULL;

        global_config.bootstraps[i] = node;
    }

    cfg_free(cfg);
}

