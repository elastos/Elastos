#ifndef __ELA_PFD_CONFIG_H__
#define __ELA_PFD_CONFIG_H__

#include <ela_carrier.h>
#include <ela_session.h>
#include <linkedhashtable.h>

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
    bool udp_enabled;

    size_t bootstraps_size;
    BootstrapNode **bootstraps;

    int loglevel;
    char *logfile;

    char *datadir;

    int mode;
    int options;
    char *serverid;
    char *server_address;

    hashtable_t *services;
    hashtable_t *users;
} PFConfig;

PFConfig *load_config(const char *config_file);

#endif /* __ELA_PFD_CONFIG_H__ */
