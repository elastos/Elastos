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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libconfig.h>

#include "rc_mem.h"
#include "config.h"

TestConfig global_config;

static int get_str(config_t *cfg, const char *name, const char *default_value,
                   char *value, size_t len)
{
    const char *stropt;
    int rc;

    rc = config_lookup_string(cfg, name, &stropt);
    if (!rc) {
        if (!default_value) {
            fprintf(stderr, "Missing '%s' option.\n", name);
            config_destroy(cfg);
            exit(-1);
        } else {
            stropt = default_value;
        }
    }

    if (strlen(stropt) >= len) {
        fprintf(stderr, "Option '%s' too long.\n", name);
        config_destroy(cfg);
        exit(-1);
    }

    strcpy(value, stropt);
    return 0;
}

static int get_int(config_t *cfg, const char *name, int default_value)
{
    int intopt;
    int rc;

    rc = config_lookup_int(cfg, name, &intopt);
    return rc ? intopt : default_value;
}

static void qualified_path(const char *path, const char *ref, char *qualified)
{
    if (*path == '/') {
        strcpy(qualified, path);
    } else if (*path == '~') {
        sprintf(qualified, "%s%s", getenv("HOME"), path+1);
    } else {
        if (ref) {
            const char *p = strrchr(ref, '/');
            if (!p) p = ref;

            if (p - ref > 0)
                strncpy(qualified, ref, p - ref);
            else
                *qualified = 0;
        } else {
            getcwd(qualified, PATH_MAX);
        }

        if (*qualified)
            strcat(qualified, "/");

        strcat(qualified, path);
    }
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

    free(config->bootstraps);
}

void load_config(const char *config_file)
{
    config_t cfg;
    config_setting_t *bootstraps;
    char *stropt;
    char number[64];
    char path[PATH_MAX];
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
        free_bootstraps(&global_config);
        exit(-1);
    }

    global_config.shuffle = (int)get_int(&cfg, "shuffle", 1);
    global_config.log2file = (int)get_int(&cfg, "log2file", 0);

    get_str(&cfg, "datadir", "~/.wtests", path, sizeof(path));
    qualified_path(path, config_file, global_config.data_location);

    global_config.tests.loglevel = (int)get_int(&cfg, "tests.loglevel", 4);
    global_config.robot.loglevel = (int)get_int(&cfg, "robot.loglevel", 4);

    bootstraps = config_lookup(&cfg, "bootstraps");
    if (!bootstraps) {
        fprintf(stderr, "Missing bootstraps section.\n");
        config_destroy(&cfg);
        free_bootstraps(&global_config);
        exit(-1);
    }

    entries = config_setting_length(bootstraps);
    if (entries <= 0) {
        fprintf(stderr, "Empty bootstraps option.\n");
        config_destroy(&cfg);
        free_bootstraps(&global_config);
        exit(-1);
    }

    global_config.bootstraps_size = entries;
    global_config.bootstraps = (BootstrapNode **)calloc(1, global_config.bootstraps_size *
                                                  sizeof(BootstrapNode *));
    if (!global_config.bootstraps) {
        fprintf(stderr, "Out of memory.\n");
        config_destroy(&cfg);
        free_bootstraps(&global_config);
        exit(-1);
    }

    for (i = 0; i < entries; i++) {
        BootstrapNode *node;

        node = rc_zalloc(sizeof(BootstrapNode), bootstrap_destroy);
        if (!node) {
            fprintf(stderr, "Out of memory.\n");
            config_destroy(&cfg);
            free_bootstraps(&global_config);
            exit(-1);
        }

        config_setting_t *bs = config_setting_get_elem(bootstraps, i);

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

        global_config.bootstraps[i] = node;
    }

    config_destroy(&cfg);
}

