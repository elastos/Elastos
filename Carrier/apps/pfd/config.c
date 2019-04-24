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

#include "carrier_config.h"
#include "config.h"

static void extra_config_free(PFConfig *config)
{
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

static int extra_config_parser(void *p, ElaOptions *options)
{
    config_t *cfg = (config_t *)p;
    PFConfig *config = (PFConfig *)options;

    config_setting_t *setting;
    const char *stropt;
    char number[64];
    int intopt;
    int entries;
    int i;
    int rc;

    rc = config_lookup_string(cfg, "mode", &stropt);
    if (!rc || !*stropt) {
        fprintf(stderr, "Missing mode option.\n");
        extra_config_free(config);
        return -1;
    }

    if (strcmp(stropt, "client") == 0) {
        config->mode = MODE_CLIENT;
    } else if (strcmp(stropt, "server") == 0) {
        config->mode = MODE_SERVER;
    } else {
        fprintf(stderr, "Unknown mode '%s'.\n", stropt);
        extra_config_free(config);
        return -1;
    }

    if (config->mode == MODE_CLIENT) {
        rc = config_lookup_string(cfg, "server", &stropt);
        if (!rc || !*stropt) {
            fprintf(stderr, "Missing server option.\n");
            extra_config_free(config);
            return -1;
        }
        config->serverid = strdup(stropt);

        rc = config_lookup_string(cfg, "server-address", &stropt);
        if (!rc || !*stropt) {
            fprintf(stderr, "Missing server_address option.\n");
            extra_config_free(config);
            return -1;
        }
        config->server_address = strdup(stropt);
    }

    rc = config_lookup_bool(cfg, "plain", &intopt);
    if (rc && intopt)
        config->options |= ELA_STREAM_PLAIN;
    config->options |= ELA_STREAM_RELIABLE;

    setting = config_lookup(cfg, "services");
    if (!setting) {
        fprintf(stderr, "Missing services section.\n");
        extra_config_free(config);
        return -1;
    }

    entries = config_setting_length(setting);
    if (entries <= 0) {
        fprintf(stderr, "Empty services option.\n");
        extra_config_free(config);
        return -1;
    }

    config->services = hashtable_create(entries * 2, 0, NULL, NULL);
    if (!config->services) {
        fprintf(stderr, "Out of memory.\n");
        extra_config_free(config);
        return -1;
    }

    for (i = 0; i < entries; i++) {
        PFService *svc = rc_zalloc(sizeof(PFService), service_destructor);
        if (!svc) {
            fprintf(stderr, "Out of memory.\n");
            extra_config_free(config);
            return -1;
        }

        config_setting_t *service = config_setting_get_elem(setting, i);

        const char *name = config_setting_name(service);
        if (name)
            svc->name = strdup(name);
        else {
            fprintf(stderr, "Missing service name.\n");
            deref(svc);
            extra_config_free(config);
            return -1;
        }

        rc = config_setting_lookup_string(service, "host", &stropt);
        if (!rc || !*stropt)
            stropt = "127.0.0.1";
        svc->host = strdup(stropt);

        rc = config_setting_lookup_int(service, "port", &intopt);
        if (!rc || !intopt) {
            fprintf(stderr, "Missing port option for service");
            deref(svc);
            extra_config_free(config);
            return -1;
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
        setting = config_lookup(cfg, "users");
        if (!setting) {
            fprintf(stderr, "Missing users option.\n");
            extra_config_free(config);
            return -1;
        }

        entries = config_setting_length(setting);
        if (entries <= 0) {
            fprintf(stderr, "Enpty users option.\n");
            extra_config_free(config);
            return -1;
        }

        config->users = hashtable_create(entries * 2, 0, NULL, NULL);
        if (!config->users) {
            fprintf(stderr, "Load configuration failed, out of memory.\n");
            extra_config_free(config);
            return -1;
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
                extra_config_free(config);
                return -1;
            }

            if (!ela_id_is_valid(stropt)) {
                fprintf(stderr, "User id '%s' is invalid in users list", stropt);
                extra_config_free(config);
                return -1;
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
                extra_config_free(config);
                return -1;
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

    return 0;
}

PFConfig *load_config(const char *config_file, PFConfig *config)
{
    memset(config, 0, sizeof(PFConfig));

    return (PFConfig *)carrier_config_load(config_file, extra_config_parser,
                (ElaOptions *)config);
}

void free_config(PFConfig *config)
{
    if (!config)
        return;

    extra_config_free(config);

    carrier_config_free(&(config->ela_options));

    memset(config, 0, sizeof(PFConfig));
}
