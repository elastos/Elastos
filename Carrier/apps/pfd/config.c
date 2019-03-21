/*
 * Copyright (c) 2018 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_DIRECT_H
#include <direct.h>
#endif

#include <libconfig.h>
#include <crystal.h>


#include <ela_carrier.h>
#include <ela_session.h>

#include "config.h"

static void config_destructor(void *p)
{
    PFConfig *config = (PFConfig *)p;

    if (!config)
        return;

    if (config->bootstraps) {
        int i;

        for (i = 0; i < (int)config->bootstraps_size; i++)
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

static void service_destructor(void *p)
{
    PFService *svc = (PFService *)p;

    if (!svc)
        return;

    if (svc->host)
        free(svc->host);

    if (svc->port)
        free(svc->port);
}

static void user_destructor(void *p)
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

static void bootstrap_destructor(void *p)
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

#if defined(_WIN32) || defined(_WIN64)
#define PATH_SEP        "\\"
#define HOME_ENV        "USERPROFILE"
#else
#define PATH_SEP        "/"
#define HOME_ENV        "HOME"
#endif

static void qualified_path(const char *path, const char *ref, char *qualified)
{
    if (*path == PATH_SEP[0] || path[1] == ':') {
        strcpy(qualified, path);
    } else if (*path == '~') {
        sprintf(qualified, "%s%s", getenv(HOME_ENV), path+1);
    } else {
        if (ref) {
            const char *p = strrchr(ref, PATH_SEP[0]);
            if (!p) p = ref;

            if (p - ref > 0)
                strncpy(qualified, ref, p - ref);
            else
                *qualified = 0;
        } else {
            getcwd(qualified, PATH_MAX);
        }

        if (*qualified)
            strcat(qualified, PATH_SEP);

        strcat(qualified, path);
    }
}

PFConfig *load_config(const char *config_file)
{
    PFConfig *config;
    config_t cfg;
    config_setting_t *setting;
    const char *stropt;
    char number[64];
    int intopt;
    int entries;
    int i;
    int rc;

    config_init(&cfg);

    rc = config_read_file(&cfg, config_file);
    if (!rc) {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
                config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
        return NULL;
    }

    config = (PFConfig *)rc_zalloc(sizeof(PFConfig), config_destructor);
    if (!config) {
        fprintf(stderr, "Load configuration failed, out of memory.\n");
        config_destroy(&cfg);
        return NULL;
    }

    rc = config_lookup_bool (&cfg, "udp_enabled", &intopt);
    if (rc && intopt) {
        config->udp_enabled = true;
    }

    setting = config_lookup(&cfg, "bootstraps");
    if (!setting) {
        fprintf(stderr, "Missing bootstraps section.\n");
        config_destroy(&cfg);
        deref(config);
        return NULL;
    }

    entries = config_setting_length(setting);
    if (entries <= 0) {
        fprintf(stderr, "Empty bootstraps option.\n");
        config_destroy(&cfg);
        deref(config);
        return NULL;
    }

    config->bootstraps_size = entries;
    config->bootstraps = (BootstrapNode **)calloc(1, config->bootstraps_size *
                                                  sizeof(BootstrapNode *));
    if (!config->bootstraps) {
        fprintf(stderr, "Out of memory.\n");
        config_destroy(&cfg);
        deref(config);
        return NULL;
    }

    for (i = 0; i < entries; i++) {
        BootstrapNode *node;

        node = rc_zalloc(sizeof(BootstrapNode), bootstrap_destructor);
        if (!node) {
            fprintf(stderr, "Out of memory.\n");
            config_destroy(&cfg);
            deref(config);
            return NULL;
        }

        config_setting_t *bs = config_setting_get_elem(setting, i);

        rc = config_setting_lookup_string(bs, "ipv4", &stropt);
        if (rc && *stropt)
            node->ipv4 = (const char *)strdup(stropt);
        else
            node->ipv4 = NULL;

        rc = config_setting_lookup_string(bs, "ipv6", &stropt);
        if (rc && *stropt)
            node->ipv6 = (const char *)strdup(stropt);
        else
            node->ipv6 = NULL;

        rc = config_setting_lookup_int(bs, "port", &intopt);
        if (rc && intopt) {
            sprintf(number, "%d", intopt);
            node->port = (const char *)strdup(number);
        } else
            node->port = NULL;

        rc = config_setting_lookup_string(bs, "public_key", &stropt);
        if (rc && *stropt)
            node->public_key = (const char *)strdup(stropt);
        else
            node->public_key = NULL;

        config->bootstraps[i] = node;
    }

    config->loglevel = 3;
    config_lookup_int(&cfg, "loglevel", &config->loglevel);

    rc = config_lookup_string(&cfg, "logfile", &stropt);
    if (rc && *stropt) {
        config->logfile = strdup(stropt);
    }

    rc = config_lookup_string(&cfg, "datadir", &stropt);
    if (!rc || !*stropt) {
        fprintf(stderr, "Missing datadir option.\n");
        config_destroy(&cfg);
        deref(config);
        return NULL;
    } else {
        char path[PATH_MAX];
        qualified_path(stropt, config_file, path);
        config->datadir = strdup(path);
    }

    rc = config_lookup_string(&cfg, "mode", &stropt);
    if (!rc || !*stropt) {
        fprintf(stderr, "Missing mode option.\n");
        config_destroy(&cfg);
        deref(config);
        return NULL;
    }

    if (strcmp(stropt, "client") == 0) {
        config->mode = MODE_CLIENT;
    } else if (strcmp(stropt, "server") == 0) {
        config->mode = MODE_SERVER;
    } else {
        fprintf(stderr, "Unknown mode '%s'.\n", stropt);
        config_destroy(&cfg);
        deref(config);
        return NULL;
    }

    if (config->mode == MODE_CLIENT) {
        rc = config_lookup_string(&cfg, "server", &stropt);
        if (!rc || !*stropt) {
            fprintf(stderr, "Missing server option.\n");
            config_destroy(&cfg);
            deref(config);
            return NULL;
        }
        config->serverid = strdup(stropt);

        rc = config_lookup_string(&cfg, "server_address", &stropt);
        if (!rc || !*stropt) {
            fprintf(stderr, "Missing server_address option.\n");
            config_destroy(&cfg);
            deref(config);
            return NULL;
        }
        config->server_address = strdup(stropt);
    }

    rc = config_lookup_bool(&cfg, "plain", &intopt);
    if (rc && intopt)
        config->options |= ELA_STREAM_PLAIN;
    config->options |= ELA_STREAM_RELIABLE;

    setting = config_lookup(&cfg, "services");
    if (!setting) {
        fprintf(stderr, "Missing services section.\n");
        config_destroy(&cfg);
        deref(config);
        return NULL;
    }

    entries = config_setting_length(setting);
    if (entries <= 0) {
        fprintf(stderr, "Empty services option.\n");
        config_destroy(&cfg);
        deref(config);
        return NULL;
    }

    config->services = hashtable_create(entries * 2, 0, NULL, NULL);
    if (!config->services) {
        fprintf(stderr, "Out of memory.\n");
        config_destroy(&cfg);
        deref(config);
        return NULL;
    }

    for (i = 0; i < entries; i++) {
        PFService *svc = rc_zalloc(sizeof(PFService), service_destructor);
        if (!svc) {
            fprintf(stderr, "Out of memory.\n");
            config_destroy(&cfg);
            deref(config);
            return NULL;
        }

        config_setting_t *service = config_setting_get_elem(setting, i);

        const char *name = config_setting_name(service);
        if (name)
            svc->name = strdup(name);
        else {
            fprintf(stderr, "Missing service name.\n");
            deref(svc);
            config_destroy(&cfg);
            deref(config);
            return NULL;
        }

        rc = config_setting_lookup_string(service, "host", &stropt);
        if (!rc || !*stropt)
            stropt = "127.0.0.1";
        svc->host = strdup(stropt);

        rc = config_setting_lookup_int(service, "port", &intopt);
        if (!rc || !intopt) {
            fprintf(stderr, "Missing port option for service");
            deref(svc);
            config_destroy(&cfg);
            deref(config);
            return NULL;
        }
        sprintf(number, "%d", intopt);
        svc->port = strdup(number);

        svc->he.key = svc->name;
        svc->he.keylen = strlen(svc->name);
        svc->he.data = svc;

        hashtable_put(config->services, &svc->he);
        deref(svc);
    }

    if (config->mode == MODE_SERVER) {
        setting = config_lookup(&cfg, "users");
        if (!setting) {
            fprintf(stderr, "Missing users option.\n");
            config_destroy(&cfg);
            deref(config);
            return NULL;
        }

        entries = config_setting_length(setting);
        if (entries <= 0) {
            fprintf(stderr, "Enpty users option.\n");
            config_destroy(&cfg);
            deref(config);
            return NULL;
        }

        config->users = hashtable_create(entries * 2, 0, NULL, NULL);
        if (!config->users) {
            fprintf(stderr, "Load configuration failed, out of memory.\n");
            config_destroy(&cfg);
            deref(config);
            return NULL;
        }

        for (i = 0; i < entries; i++) {
            int j;
            int nsvcs;
            PFUser *user;
            config_setting_t *usetting, *svcs;

            usetting = config_setting_get_elem(setting, i);

            rc = config_setting_lookup_string(usetting, "user", &stropt);
            if (!rc) {
                fprintf(stderr, "Missing user id option in users list.\n");
                config_destroy(&cfg);
                deref(config);
                return NULL;
            }

            if (!ela_id_is_valid(stropt)) {
                fprintf(stderr, "User id '%s' is invalid in users list", stropt);
                config_destroy(&cfg);
                deref(config);
                return NULL;
            }

            svcs = config_setting_get_member(usetting, "services");
            if (!svcs) // No allowed service for current user
                continue;

            nsvcs = config_setting_length(svcs);
            if (nsvcs <= 0) // No allowed service for current user
                continue;

            user = rc_zalloc(sizeof(PFUser) + (nsvcs + 1) * sizeof(char *), user_destructor);
            if (!user) {
                fprintf(stderr, "Load configuration failed, out of memory.\n");
                config_destroy(&cfg);
                deref(config);
                return NULL;
            }

            user->userid = strdup(stropt);

            for (j = 0; j < nsvcs; j++) {
                stropt = config_setting_get_string_elem(svcs, j);
                user->services[j] = strdup(stropt);
            }

            user->he.key = user->userid;
            user->he.keylen = strlen(user->userid);
            user->he.data = user;

            hashtable_put(config->users, &user->he);
            deref(user);
        }
    }

    config_destroy(&cfg);
    return config;
}
