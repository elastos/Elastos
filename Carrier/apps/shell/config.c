#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>

#include <confuse.h>
#include <rc_mem.h>

#include "config.h"

static void config_error(cfg_t *cfg, const char *fmt, va_list ap)
{
    fprintf(stderr, "Config file error, line %d: ", cfg->line);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
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

    if (cfg_getstr(sec, "address") == NULL) {
        cfg_error(cfg, "address option missing");
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

static int not_null_validator(cfg_t *cfg, cfg_opt_t *opt)
{
    if (cfg_getstr(cfg, opt->name) == NULL) {
        cfg_error(cfg, "option '%s' missing.", opt->name);
        return -1;
    }

    return 0;
}

static void config_destroy(void *p)
{
    ShellConfig *config = (ShellConfig *)p;

    if (!config)
        return;

    if (config->logfile)
        free(config->logfile);

    if (config->datadir)
        free(config->datadir);

    if (config->bootstraps) {
        int i;

        for (i = 0; i < config->bootstraps_size; i++)
            deref(config->bootstraps[i]);

        free(config->bootstraps);
    }
}

static void bootstrap_destroy(void *p)
{
    BootstrapNode *node = (BootstrapNode *)p;

    if (!node)
        return;

    if (node->ipv4)
        free(node->ipv4);

    if (node->ipv6)
        free(node->ipv6);

    if (node->port)
        free(node->port);

    if (node->address)
        free(node->address);
}

ShellConfig *load_config(const char *config_file)
{
    ShellConfig *config;
    cfg_t *cfg, *bootstraps, *sec;
    const char *stropt;
    int nsecs;
    int i;
    int rc;

    cfg_opt_t bootstrap_opts[] = {
        CFG_STR("ipv4", NULL, CFGF_NONE),
        CFG_STR("ipv6", NULL, CFGF_NONE),
        CFG_STR("port", "33445", CFGF_NONE),
        CFG_STR("address", NULL, CFGF_NONE),
        CFG_END()
    };

    cfg_opt_t bootstraps_opts[] = {
        CFG_SEC("bootstrap", bootstrap_opts, CFGF_MULTI | CFGF_NO_TITLE_DUPES),
    };

    cfg_opt_t cfg_opts[] = {
        CFG_BOOL("udp_enabled", true, CFGF_NONE),
        CFG_INT("loglevel", 3, CFGF_NONE),
        CFG_STR("logfile", NULL, CFGF_NONE),
        CFG_STR("datadir", NULL, CFGF_NONE),
        CFG_SEC("bootstraps", bootstraps_opts, CFGF_NONE),
        CFG_END()
    };

    cfg = cfg_init(cfg_opts, CFGF_NONE);
    cfg_set_error_function(cfg, config_error);
    cfg_set_validate_func(cfg, NULL, not_null_validator);
    cfg_set_validate_func(cfg, "bootstraps", bootstraps_validator);
    cfg_set_validate_func(cfg, "bootstraps|bootstrap", bootstrap_validator);

    rc = cfg_parse(cfg, config_file);
    if (rc != CFG_SUCCESS) {
        cfg_error(cfg, "can not pase config file: %s.", config_file);
        cfg_free(cfg);
        return NULL;
    }

    config = (ShellConfig *)rc_zalloc(sizeof(ShellConfig), config_destroy);
    if (!config) {
        cfg_error(cfg, "out of memory.");
        cfg_free(cfg);
        return NULL;
    }

    config->udp_enabled = cfg_getbool(cfg, "udp_enabled");

    config->loglevel = (int)cfg_getint(cfg, "loglevel");

    stropt = cfg_getstr(cfg, "logfile");
    if (stropt)
        config->logfile = strdup(stropt);

    stropt = cfg_getstr(cfg, "datadir");
    if (!stropt) {
        char datadir[PATH_MAX];

        sprintf(datadir, "%s/%s", getenv("HOME"), ".wshell");
        config->datadir = strdup(datadir);
    } else
        config->datadir = strdup(stropt);

    bootstraps = cfg_getsec(cfg, "bootstraps");
    if (!bootstraps) {
        cfg_error(cfg, "missing services section.");
        cfg_free(cfg);
        deref(config);
        return NULL;
    }

    nsecs = cfg_size(bootstraps, "bootstrap");

    config->bootstraps_size = nsecs;
    config->bootstraps = (BootstrapNode **)calloc(1, config->bootstraps_size *
                                                  sizeof(BootstrapNode *));
    if (!config->bootstraps) {
        cfg_error(cfg, "out of memory.");
        cfg_free(cfg);
        deref(config);
        return NULL;
    }

    for (i = 0; i < nsecs; i++) {
        BootstrapNode *node;

        node = rc_zalloc(sizeof(BootstrapNode), bootstrap_destroy);
        if (!node) {
            cfg_error(cfg, "out of memory.");
            cfg_free(cfg);
            deref(config);
            return NULL;
        }

        sec = cfg_getnsec(bootstraps, "bootstrap", i);

        stropt = cfg_getstr(sec, "ipv4");
        if (stropt)
            node->ipv4 = strdup(stropt);
        else
            node->ipv4 = NULL;

        stropt = cfg_getstr(sec, "ipv6");
        if (stropt)
            node->ipv6 = strdup(stropt);
        else
            node->ipv6 = NULL;

        stropt = cfg_getstr(sec, "port");
        if (stropt)
            node->port = strdup(stropt);
        else
            node->port = NULL;

        stropt = cfg_getstr(sec, "address");
        if (stropt)
            node->address = strdup(stropt);
        else
            node->address = NULL;

        config->bootstraps[i] = node;
    }

    return config;
}

