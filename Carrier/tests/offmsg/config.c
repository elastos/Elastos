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
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_DIRECT_H
#include <direct.h>
#endif

#include <libconfig.h>
#include <crystal.h>

#include "carrier_config.h"
#include "config.h"

static int extra_config_parser(void *p, ElaOptions *options)
{
    config_t *cfg = (config_t *)p;
    TestConfig *config = (TestConfig *)options;

    config_lookup_int(cfg, "sender.loglevel", &config->sender_log_level);
    config_lookup_int(cfg, "receiver.loglevel", &config->receiver_log_level);

    return 0;
}

TestConfig *load_config(const char *config_file, TestConfig *config)
{
    memset(config, 0, sizeof(TestConfig));

    return (TestConfig *)carrier_config_load(config_file, extra_config_parser,
                (ElaOptions *)config);
}

void free_config(TestConfig *config)
{
    if (!config)
        return;

    carrier_config_free(&(config->shared_options));

    memset(config, 0, sizeof(TestConfig));
}
