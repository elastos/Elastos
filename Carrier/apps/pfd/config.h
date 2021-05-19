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

#ifndef __ELA_PFD_CONFIG_H__
#define __ELA_PFD_CONFIG_H__

#include <ela_carrier.h>
#include <ela_session.h>
#include <crystal.h>

#define MODE_CLIENT     1
#define MODE_SERVER     2

typedef struct {
    hash_entry_t he;
    char *name;
    char *host;
    char *port;
} PFService;

typedef struct {
    hash_entry_t he;
    char *userid;
    char *services[0];
} PFUser;

typedef struct {
    ElaOptions ela_options;

    int mode;
    int options;
    char *serverid;
    char *server_address;

    hashtable_t *services;
    hashtable_t *users;
} PFConfig;

PFConfig *load_config(const char *config_file, PFConfig *config);
void free_config(PFConfig *config);

#endif /* __ELA_PFD_CONFIG_H__ */
