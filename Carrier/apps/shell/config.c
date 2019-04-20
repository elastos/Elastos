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


#include "config.h"

static void config_destructor(void *p)
{
    ShellConfig *config = (ShellConfig *)p;

    if (!config)
        return;

    if (config->logfile)
        free(config->logfile);

    if (config->datadir)
        free(config->datadir);

    if (config->dht_bootstraps) {
        int i;

        for (i = 0; i < config->dht_bootstraps_size; i++)
            deref(config->dht_bootstraps[i]);

        free(config->dht_bootstraps);
    }

    if (config->hive_bootstraps) {
        int i;

        for (i = 0; i < config->hive_bootstraps_size; i++)
            deref(config->hive_bootstraps[i]);

        free(config->hive_bootstraps);
    }
}

static void dht_bootstrap_destructor(void *p)
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

static void hive_bootstrap_destructor(void *p)
{
    HiveBootstrapNode *node = (HiveBootstrapNode *)p;

    if (!node)
        return;

    if (node->ipv4)
        free((void *)node->ipv4);

    if (node->ipv6)
        free((void *)node->ipv6);

    if (node->port)
        free((void *)node->port);
}

static void qualified_path(const char *path, const char *ref, char *qualified)
{
    if (*path == '/') {
        strcpy(qualified, path);
    } else if (*path == '~') {
        sprintf(qualified, "%s%s", getenv("HOME"), path+1);
    } else {
        sprintf(qualified, "%s/%s", getenv("PWD"), path);
    }
}

ShellConfig *load_config(const char *config_file)
{
    ShellConfig *config;
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

    config = (ShellConfig *)rc_zalloc(sizeof(ShellConfig), config_destructor);
    if (!config) {
        fprintf(stderr, "Load configuration failed, out of memory.\n");
        config_destroy(&cfg);
        return NULL;
    }

    rc = config_lookup_bool(&cfg, "udp_enabled", &intopt);
    if (rc && intopt) {
        config->udp_enabled = true;
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
        char path[PATH_MAX] = {0};
        qualified_path(stropt, config_file, path);
        config->datadir = strdup(path);
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

    config->dht_bootstraps_size = entries;
    config->dht_bootstraps = (BootstrapNode **)calloc(1, config->dht_bootstraps_size *
                                                  sizeof(BootstrapNode *));
    if (!config->dht_bootstraps) {
        fprintf(stderr, "Out of memory.\n");
        config_destroy(&cfg);
        deref(config);
        return NULL;
    }

    for (i = 0; i < entries; i++) {
        BootstrapNode *node;

        node = rc_zalloc(sizeof(BootstrapNode), dht_bootstrap_destructor);
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

        config->dht_bootstraps[i] = node;
    }

    setting = config_lookup(&cfg, "hive_bootstraps");
    if (!setting) {
        fprintf(stderr, "Missing hive_bootstraps section.\n");
        config_destroy(&cfg);
        deref(config);
        return NULL;
    }

    entries = config_setting_length(setting);
    if (entries <= 0) {
        fprintf(stderr, "Empty hive_bootstraps option.\n");
        config_destroy(&cfg);
        deref(config);
        return NULL;
    }

    config->hive_bootstraps_size = entries;
    config->hive_bootstraps = (HiveBootstrapNode **)calloc(1, config->hive_bootstraps_size *
                                                           sizeof(HiveBootstrapNode *));
    if (!config->hive_bootstraps) {
        fprintf(stderr, "Out of memory.\n");
        config_destroy(&cfg);
        deref(config);
        return NULL;
    }

    for (i = 0; i < entries; i++) {
        HiveBootstrapNode *node;

        node = rc_zalloc(sizeof(HiveBootstrapNode), hive_bootstrap_destructor);
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

        config->hive_bootstraps[i] = node;
    }

    config_destroy(&cfg);
    return config;
}

