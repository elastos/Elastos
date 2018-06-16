#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>

#include <confuse.h>
#include <rc_mem.h>

#include <ela_carrier.h>

#include "config.h"

static void config_error(cfg_t *cfg, const char *fmt, va_list ap)
{
    fprintf(stderr, "Config file error, line %d: ", cfg->line);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
}

static int mode_validator(cfg_t *cfg, cfg_opt_t *opt)
{
    const char *stropt = cfg_opt_getnstr(opt, cfg_opt_size(opt) - 1);
    if (stropt == NULL) {
        cfg_error(cfg, "Missing mode option");
        return -1;
    }

    if (strcmp(stropt, "client") != 0 && strcmp(stropt, "server") != 0) {
        cfg_error(cfg, "Unknown mode '%s'", stropt);
        return -1;
    }

    return 0;
}

static int service_validator(cfg_t *cfg, cfg_opt_t *opt)
{
    cfg_t *sec;

    sec = cfg_opt_getnsec(opt, cfg_opt_size(opt) - 1);

    if (cfg_getstr(sec, "port") == NULL) {
        cfg_error(cfg, "port option missing");
        return -1;
    }

    return 0;
}

static int services_validator(cfg_t *cfg, cfg_opt_t *opt)
{
    int nsvcs;

    nsvcs = cfg_size(cfg, "services|service");

    if (nsvcs == 0) {
        cfg_error(cfg, "no service defined.");
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
    PFConfig *config = (PFConfig *)p;

    if (!config)
        return;

    if (config->bootstraps) {
        int i;

        for (i = 0; i < config->bootstraps_size; i++)
            deref(config->bootstraps[i]);

        free(config->bootstraps);
    }

    if (config->logfile)
        free(config->logfile);

    if (config->datadir)
        free(config->datadir);

    if (config->serverid)
        free(config->serverid);

    if (config->server_address)
        free(config->server_address);

    if (config->services)
        deref(config->services);

    if (config->users)
        deref(config->users);
}

static void service_destroy(void *p)
{
    PFService *svc = (PFService *)p;

    if (!svc)
        return;

    if (svc->host)
        free(svc->host);

    if (svc->port)
        free(svc->port);
}

static void user_destroy(void *p)
{
    PFUser *user = (PFUser *)p;
    char **s;

    if (!user)
        return;

    if (user->userid)
        free(user->userid);

    for (s = user->services; *s != NULL; s++)
        free(*s);
}

static void bootstrap_destroy(void *p)
{
    BootstrapNode *node = (BootstrapNode *)p;

    if (!node)
        return;

    if (node->ipv4)
        free((void*)node->ipv4);

    if (node->ipv6)
        free((void*)node->ipv6);

    if (node->port)
        free((void*)node->port);

    if (node->public_key)
        free((void*)node->public_key);
}

PFConfig *load_config(const char *config_file)
{
    PFConfig *config;
    cfg_t *cfg, *svcs, *users, *sec;
    cfg_t *bootstraps;
    const char *stropt;
    int nsecs;
    int i;
    int rc;
    bool is_plain;

    cfg_opt_t service_opts[] = {
        CFG_STR("host", "127.0.0.1", CFGF_NONE),
        CFG_STR("port", NULL, CFGF_NODEFAULT),
        CFG_END()
    };

    cfg_opt_t services_opts[] = {
        CFG_SEC("service", service_opts, CFGF_TITLE | CFGF_MULTI | CFGF_NO_TITLE_DUPES),
    };

    cfg_opt_t user_opts[] = {
        CFG_STR_LIST("services", NULL, CFGF_NONE),
        CFG_END()
    };

    cfg_opt_t users_opts[] = {
        CFG_SEC("user", user_opts, CFGF_TITLE | CFGF_MULTI | CFGF_NO_TITLE_DUPES),
    };

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
        CFG_BOOL("udp_enabled", true, CFGF_NONE),
        CFG_SEC("bootstraps", bootstraps_opts, CFGF_NONE),
        CFG_INT("loglevel", 3, CFGF_NONE),
        CFG_STR("logfile", NULL, CFGF_NONE),
        CFG_STR("datadir", NULL, CFGF_NONE),
        CFG_STR("mode", NULL, CFGF_NODEFAULT),
        CFG_BOOL("plain", 0, CFGF_NONE),
        CFG_STR("server", NULL, CFGF_NONE),
        CFG_STR("server_address", NULL, CFGF_NONE),
        CFG_SEC("services", services_opts, CFGF_NONE),
        CFG_SEC("users", users_opts, CFGF_NONE),
        CFG_END()
    };

    cfg = cfg_init(cfg_opts, CFGF_NONE);
    cfg_set_error_function(cfg, config_error);
    cfg_set_validate_func(cfg, NULL, not_null_validator);
    cfg_set_validate_func(cfg, "mode", mode_validator);
    cfg_set_validate_func(cfg, "services", services_validator);
    cfg_set_validate_func(cfg, "services|service", service_validator);

    rc = cfg_parse(cfg, config_file);
    if (rc != CFG_SUCCESS) {
        cfg_error(cfg, "can not parse config file: %s.", config_file);
        cfg_free(cfg);
        return NULL;
    }

    config = (PFConfig *)rc_zalloc(sizeof(PFConfig), config_destroy);
    if (!config) {
        cfg_error(cfg, "out of memory.");
        cfg_free(cfg);
        return NULL;
    }

    config->udp_enabled = cfg_getbool(cfg, "udp_enabled");

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

        config->bootstraps[i] = node;
    }

    config->loglevel = (int)cfg_getint(cfg, "loglevel");

    stropt = cfg_getstr(cfg, "logfile");
    if (stropt) {
        config->logfile = strdup(stropt);
    }

    stropt = cfg_getstr(cfg, "datadir");
    if (!stropt) {
        char datadir[PATH_MAX];

        sprintf(datadir, "%s/%s/%s", getenv("HOME"), ".wpfd",
                config->mode == MODE_CLIENT ? "client" : "server");
        config->datadir = strdup(datadir);
    } else
        config->datadir = strdup(stropt);


    stropt = cfg_getstr(cfg, "mode");
    if (!stropt) {
        cfg_error(cfg, "missing mode option.");
        cfg_free(cfg);
        deref(config);
        return NULL;
    }

    is_plain = cfg_getbool(cfg, "plain");
    if (is_plain)
        config->options |= ELA_STREAM_PLAIN;
    config->options |= ELA_STREAM_RELIABLE;

    if (strcmp(stropt, "client") == 0) {
        config->mode = MODE_CLIENT;
    } else if (strcmp(stropt, "server") == 0) {
        config->mode = MODE_SERVER;
    } else {
        cfg_error(cfg, "unknown mode '%s'", stropt);
        cfg_free(cfg);
        deref(config);
        return NULL;
    }

    stropt = cfg_getstr(cfg, "server");
    if (!stropt) {
        if (config->mode == MODE_CLIENT) {
            cfg_error(cfg, "missing server option");
            cfg_free(cfg);
            deref(config);
            return NULL;
        }
    } else {
        if (config->mode == MODE_SERVER) {
            cfg_error(cfg, "server option not avaliable");
            cfg_free(cfg);
            deref(config);
            return NULL;
        }

        config->serverid = strdup(stropt);
    }

    stropt = cfg_getstr(cfg, "server_address");
    if (!stropt) {
        if (config->mode == MODE_CLIENT) {
            cfg_error(cfg, "missing server option");
            cfg_free(cfg);
            deref(config);
            return NULL;
        }
    } else {
        if (config->mode == MODE_SERVER) {
            cfg_error(cfg, "server option not avaliable");
            cfg_free(cfg);
            deref(config);
            return NULL;
        }

        config->server_address = strdup(stropt);
    }

    svcs = cfg_getsec(cfg, "services");
    if (!svcs) {
        cfg_error(cfg, "missing services section.");
        cfg_free(cfg);
        deref(config);
        return NULL;
    }

    nsecs = cfg_size(svcs, "service");
    config->services = hashtable_create(nsecs * 2, 0, NULL, NULL);
    if (!config->services) {
        cfg_error(cfg, "out of memory");
        cfg_free(cfg);
        deref(config);
        return NULL;
    }

    for (i = 0; i < nsecs; i++) {
        PFService *svc = rc_zalloc(sizeof(PFService), service_destroy);
        if (!svc) {
            cfg_error(cfg, "out of memory");
            deref(svc);
            cfg_free(cfg);
            deref(config);
            return NULL;
        }

        sec = cfg_getnsec(svcs, "service", i);

        stropt = cfg_title(sec);
        svc->name = strdup(stropt);

        stropt = cfg_getstr(sec, "host");
        if (!stropt)
            stropt = "127.0.0.1";
        svc->host = strdup(stropt);

        svc->port = cfg_getstr(sec, "port");
        if (!svc->port) {
            cfg_error(cfg, "missing port option for service");
            deref(svc);
            cfg_free(cfg);
            deref(config);
            return NULL;
        }

        svc->he.key = svc->name;
        svc->he.keylen = strlen(svc->name);
        svc->he.data = svc;

        hashtable_put(config->services, &svc->he);
        deref(svc);
    }

    if (config->mode == MODE_SERVER) {
        users = cfg_getsec(cfg, "users");

        if (!users) {
            cfg_error(cfg, "section users not missing");
            cfg_free(cfg);
            deref(config);
            return NULL;
        }

        nsecs = cfg_size(users, "user");
        config->users = hashtable_create(nsecs * 2, 0, NULL, NULL);
        if (!config->users) {
            cfg_error(cfg, "out of memory");
            cfg_free(cfg);
            deref(config);
            return NULL;
        }

        for (i = 0; i < nsecs; i++) {
            int j;
            int n;
            PFUser *user;

            sec = cfg_getnsec(users, "user", i);
            n = cfg_size(sec, "services");
            user = rc_zalloc(sizeof(PFUser) + (n + 1) * sizeof(char *), user_destroy);

            stropt = cfg_title(sec);
            if (!ela_id_is_valid(stropt)) {
                cfg_error(cfg, "user id '%s' is invalid", stropt);
                deref(user);
                cfg_free(cfg);
                deref(config);
                return NULL;
            }
            user->userid = strdup(stropt);

            for (j = 0; j < n; j++) {
                stropt = cfg_getnstr(sec, "services", j);
                user->services[j] = strdup(stropt);
            }

            user->he.key = user->userid;
            user->he.keylen = strlen(user->userid);
            user->he.data = user;

            hashtable_put(config->users, &user->he);
            deref(user);
        }
    }

    cfg_free(cfg);
    return config;
}
