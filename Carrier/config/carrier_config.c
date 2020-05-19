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
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_DIRECT_H
#include <direct.h>
#endif

#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#endif

#include <libconfig.h>
#include <crystal.h>

#include <ela_carrier.h>

#include "carrier_config.h"

#define DEFAULT_LOG_LEVEL   ElaLogLevel_Info
#define DEFAULT_DATA_DIR    "~/.carrier"

static void bootstraps_destructor(void *p)
{
    size_t i;
    size_t *size = (size_t *)p;
    BootstrapNode *nodes = (BootstrapNode *)(size + 1);

    for (i = 0; i < *size; i++) {
        BootstrapNode *node = nodes + i;

        if (node->ipv4)
            free((void *)node->ipv4);

        if (node->ipv6)
            free((void *)node->ipv6);

        if (node->port)
            free((void *)node->port);

        if (node->public_key)
            free((void *)node->public_key);
    }
}

static void express_nodes_destructor(void *p)
{
    size_t i;
    size_t *size = (size_t *)p;
    ExpressNode *nodes = (ExpressNode *)(size + 1);

    for (i = 0; i < *size; i++) {
        ExpressNode *node = nodes + i;

        if (node->ipv4)
            free((void *)node->ipv4);

        if (node->ipv6)
            free((void *)node->ipv6);

        if (node->port)
            free((void *)node->port);

        if (node->public_key)
            free((void *)node->public_key);
    }
}

#if defined(_WIN32) || defined(_WIN64)
#define PATH_SEP        "\\"
#define HOME_ENV        "LOCALAPPDATA"
#else
#define PATH_SEP        "/"
#define HOME_ENV        "HOME"
#endif

static void qualified_path(const char *path, const char *ref, char *qualified)
{
    assert(strlen(path) >= 1);

    if (*path == PATH_SEP[0] || path[1] == ':') {
        strcpy(qualified, path);
    } else if (*path == '~') {
        sprintf(qualified, "%s%s", getenv(HOME_ENV), path+1);
    } else {
        getcwd(qualified, PATH_MAX);
        strcat(qualified, PATH_SEP);
        strcat(qualified, path);
    }
}

ElaOptions *carrier_config_load(const char *config_file,
        int (*extra_config_handle)(void *cfg, ElaOptions *options),
        ElaOptions *options)
{
    config_t cfg;
    config_setting_t *nodes_setting;
    config_setting_t *node_setting;
    int entries;

    char path[PATH_MAX];
    char *ch;
    size_t *mem;

    const char *stropt;
    char number[64];
    int intopt;

    int i;
    int rc;

    if (!config_file || !*config_file || !options)
        return NULL;

    memset(options, 0, sizeof(ElaOptions));

    config_init(&cfg);

    strcpy(path, config_file);

#if defined(_WIN32) || defined(_WIN64)
    ch = strrchr(path, '\\');
    if (!ch) {
#endif
    ch = strrchr(path, '/');
#if defined(_WIN32) || defined(_WIN64)
    }
#endif

    if (ch) {
        *++ch = 0;
        if (strlen(path) > 1)
            *--ch = 0;

        config_set_include_dir(&cfg, (const char *)path);
    } else {
        config_set_include_dir(&cfg, ".");
    }

    rc = config_read_file(&cfg, config_file);
    if (!rc) {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
                config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
        return NULL;
    }

    nodes_setting = config_lookup(&cfg, "bootstraps");
    if (!nodes_setting) {
        fprintf(stderr, "Missing bootstraps section.\n");
        config_destroy(&cfg);
        return NULL;
    }

    entries = config_setting_length(nodes_setting);
    if (entries <= 0) {
        fprintf(stderr, "Empty bootstraps option.\n");
        config_destroy(&cfg);
        return NULL;
    }

    mem = (size_t *)rc_zalloc(sizeof(size_t) +
            sizeof(BootstrapNode) * entries, bootstraps_destructor);
    if (!mem) {
        fprintf(stderr, "Load configuration failed, out of memory.\n");
        config_destroy(&cfg);
        return NULL;
    }

    *mem = entries;
    options->bootstraps_size = entries;
    options->bootstraps = (BootstrapNode *)(++mem);

    for (i = 0; i < entries; i++) {
        BootstrapNode *node = options->bootstraps + i;

        node_setting = config_setting_get_elem(nodes_setting, i);

        rc = config_setting_lookup_string(node_setting, "ipv4", &stropt);
        if (rc && *stropt)
            node->ipv4 = (const char *)strdup(stropt);
        else
            node->ipv4 = NULL;

        rc = config_setting_lookup_string(node_setting, "ipv6", &stropt);
        if (rc && *stropt)
            node->ipv6 = (const char *)strdup(stropt);
        else
            node->ipv6 = NULL;

        rc = config_setting_lookup_int(node_setting, "port", &intopt);
        if (rc && intopt) {
            sprintf(number, "%d", intopt);
            node->port = (const char *)strdup(number);
        } else
            node->port = NULL;

        rc = config_setting_lookup_string(node_setting, "public-key", &stropt);
        if (rc && *stropt)
            node->public_key = (const char *)strdup(stropt);
        else
            node->public_key = NULL;
    }

    nodes_setting = config_lookup(&cfg, "express_nodes");
    if (!nodes_setting) {
        fprintf(stderr, "Missing express_nodes section.\n");
        config_destroy(&cfg);
        return NULL;
    }

    entries = config_setting_length(nodes_setting);
    if (entries <= 0) {
        fprintf(stderr, "Empty express_nodes option.\n");
        config_destroy(&cfg);
        return NULL;
    }

    mem = (size_t *)rc_zalloc(sizeof(size_t) +
            sizeof(ExpressNode) * entries, express_nodes_destructor);
    if (!mem) {
        fprintf(stderr, "Load configuration failed, out of memory.\n");
        config_destroy(&cfg);
        return NULL;
    }

    *mem = entries;
    options->express_nodes_size = entries;
    options->express_nodes = (ExpressNode *)(++mem);

    for (i = 0; i < entries; i++) {
        ExpressNode *node = options->express_nodes + i;

        node_setting = config_setting_get_elem(nodes_setting, i);

        rc = config_setting_lookup_string(node_setting, "ipv4", &stropt);
        if (rc && *stropt)
            node->ipv4 = (const char *)strdup(stropt);
        else
            node->ipv4 = NULL;

        node->ipv6 = NULL;

        rc = config_setting_lookup_int(node_setting, "port", &intopt);
        if (rc && intopt) {
            sprintf(number, "%d", intopt);
            node->port = (const char *)strdup(number);
        } else
            node->port = NULL;

        rc = config_setting_lookup_string(node_setting, "public-key", &stropt);
        if (rc && *stropt)
            node->public_key = (const char *)strdup(stropt);
        else
            node->public_key = NULL;
    }

    options->udp_enabled = true;
    rc = config_lookup_bool(&cfg, "udp-enabled", &intopt);
    if (rc)
        options->udp_enabled = !!intopt;

    options->log_level = DEFAULT_LOG_LEVEL;
    rc = config_lookup_int(&cfg, "log-level", &intopt);
    if (rc)
        options->log_level = intopt;

    rc = config_lookup_string(&cfg, "log-file", &stropt);
    if (rc && *stropt) {
        qualified_path(stropt, config_file, path);
        options->log_file = strdup(path);
    }

    rc = config_lookup_string(&cfg, "data-dir", &stropt);
    if (!rc || !*stropt)
        stropt = DEFAULT_DATA_DIR;

    qualified_path(stropt, config_file, path);
    options->persistent_location = strdup(path);

    if (extra_config_handle) {
        if (extra_config_handle(&cfg, options) < 0) {
            carrier_config_free(options);
            config_destroy(&cfg);
            return NULL;
        }
    }

    config_destroy(&cfg);
    return options;
}

ElaOptions *carrier_config_copy(ElaOptions *dest, ElaOptions *src)
{
    size_t *p;

    dest->udp_enabled = src->udp_enabled;
    dest->log_level = src->log_level;
    dest->log_printer = src->log_printer;
    dest->bootstraps_size = src->bootstraps_size;

    p = (size_t *)src->bootstraps;
    ref(--p);
    dest->bootstraps = src->bootstraps;

    if (src->persistent_location)
        dest->persistent_location = strdup(src->persistent_location);

    if (src->log_file)
        dest->log_file = strdup(src->log_file);

    return dest;
}

void carrier_config_update(ElaOptions *options, int argc, char *argv[])
{
    char path[PATH_MAX];

    int opt;
    int idx;
    struct option cmd_options[] = {
        { "udp-enabled",    required_argument,  NULL, 1 },
        { "log-level",      required_argument,  NULL, 2 },
        { "log-file",       required_argument,  NULL, 3 },
        { "data-dir",       required_argument,  NULL, 4 },
        { NULL,             0,                  NULL, 0 }
    };

    // Reset optind to 1 to rescan the same command line.
    optind = 1;
    opterr = 0;
    optopt = 0;

    // Use a plus sign('+') to avoid permuting the contents of argv while scanning it.
    while ((opt = getopt_long(argc, argv, "+c:h?", cmd_options, &idx)) != -1) {
        switch (opt) {
        case 1:
            if (isdigit(*optarg))
                options->udp_enabled = atoi(optarg) != 0;
            else
                options->udp_enabled = strcmp(optarg, "true") == 0;
            break;

        case 2:
            options->log_level = (ElaLogLevel)atoi(optarg);
            break;

        case 3:
            if (options->log_file)
                free((void *)options->log_file);

            qualified_path(optarg, NULL, path);
            options->log_file = strdup(path);
            break;

        case 4:
            if (options->persistent_location)
                free((void *)options->persistent_location);

            qualified_path(optarg, NULL, path);
            options->persistent_location = strdup(path);
            break;

        default:
            break;
        }
    }

    // Reset optind to 1 to rescan the same command line.
    optind = 1;
    opterr = 0;
    optopt = 0;
}

void carrier_config_free(ElaOptions *options)
{
    if (!options)
        return;

    if (options->persistent_location)
        free((void *)options->persistent_location);

    if (options->log_file)
        free((void *)options->log_file);

    if (options->bootstraps) {
        size_t *p = (size_t *)options->bootstraps;
        deref(--p);
    }

    if (options->express_nodes) {
        size_t *p = (size_t *)options->express_nodes;
        deref(--p);
    }

#ifndef NDEBUG
    memset(options, 0, sizeof(ElaOptions));
#endif
}

const char *get_config_file(const char *config_file, const char *default_config_files[])
{
    const char **file = config_file ? &config_file : default_config_files;

    for (; *file; ) {
        int fd = open(*file, O_RDONLY);
        if (fd < 0) {
            if (*file == config_file)
                file = default_config_files;
            else
                file++;

            continue;
        }

        close(fd);

        return *file;
    }

    return NULL;
}

