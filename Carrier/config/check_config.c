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

#include <stdio.h>
#include <string.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include <crystal.h>
#include <ela_carrier.h>

#include "carrier_config.h"

#define CONFIG_NAME     "carrier.conf"

static const char *default_config_files[] = {
    "./"CONFIG_NAME,
    "../etc/carrier/"CONFIG_NAME,
#if !defined(_WIN32) && !defined(_WIN64)
    "/usr/local/etc/carrier/"CONFIG_NAME,
    "/etc/carrier/"CONFIG_NAME,
#endif
    NULL
};

static void print_options(ElaOptions *options)
{
    size_t i;

    printf("Current configuration\n");
    printf("===============================================================\n");
    printf("%zi bootstrap nodes:\n", options->bootstraps_size);

    for (i = 0; i < options->bootstraps_size; i++) {
        BootstrapNode *node = options->bootstraps + i;

        printf("  %-44s, ", node->public_key ? node->public_key : "N/A");

        if (node->ipv4)
            printf("%s%s", node->ipv4, node->ipv6 ? " | " : ", ");
        if (node->ipv6)
            printf("%s, ", node->ipv6);

        printf("%s\n", node->port ? node->port : "N/A");
    }

    printf("UDP enabled: %s\n", options->udp_enabled ? "true" : "false");
    printf("Log level: %d\n", options->log_level);
    printf("Log file: %s\n", options->log_file ? options->log_file : "[STDOUT]");
    printf("Data dir: %s\n\n", options->persistent_location);
}

static void usage(void)
{
    printf("Elastos carrier configuration checker.\n");
    printf("Usage: checkconfig [OPTION]...\n");
    printf("\n");
    printf("Avaliable options:\n");
    printf("  -c, --config=CONFIG_FILE  Set config file path.\n");
    printf("      --udp-enabled=0|1     Enable UDP, override the option in config.\n");
    printf("      --log-level=LEVEL     Log level(0-7), override the option in config.\n");
    printf("      --log-file=FILE       Log file name, override the option in config.\n");
    printf("      --data-dir=PATH       Data location, override the option in config.\n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    ElaOptions options;
    const char *config_file = NULL;

    int opt;
    int idx;
    struct option cmd_options[] = {
        { "config",         required_argument,  NULL, 'c' },
        { "udp-enabled",    required_argument,  NULL, 1 },
        { "log-level",      required_argument,  NULL, 2 },
        { "log-file",       required_argument,  NULL, 3 },
        { "data-dir",       required_argument,  NULL, 4 },
        { "help",           no_argument,        NULL, 'h' },
        { NULL,             0,                  NULL, 0 }
    };

    memset(&options, 0, sizeof(options));

    while ((opt = getopt_long(argc, argv, "c:h?", cmd_options, &idx)) != -1) {
        switch (opt) {
        case 'c':
            config_file = optarg;
            break;

        case 1:
        case 2:
        case 3:
        case 4:
            break;

        case 'h':
        case '?':
        default:
            usage();
            return -1;
        }
    }

    config_file = get_config_file(config_file, default_config_files);
    if (!config_file) {
        printf("Missing configuration file.\n");
        printf("Usage: checkconfig [config_file]\n\n");
        return -1;
    }

    printf("Using configuration: %s\n\n", config_file);

    if (carrier_config_load(config_file, NULL, &options) == NULL)
        return -1;

    carrier_config_update(&options, argc, argv);

    print_options(&options);

    carrier_config_free(&options);
    return 0;
}