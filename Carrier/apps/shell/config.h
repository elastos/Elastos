#ifndef __SHELL_CONFIG_H__
#define __SHELL_CONFIG_H__

#include <stdbool.h>
#include <rc_mem.h>

typedef struct {
    char *ipv4;
    char *ipv6;
    char *port;
    char *address;
} BootstrapNode;

typedef struct {
    bool udp_enabled;

    int loglevel;
    char *logfile;

    char *datadir;

    int bootstraps_size;
    BootstrapNode **bootstraps;
} ShellConfig;

ShellConfig *load_config(const char *config_file);

#endif /* __SHELL_CONFIG_H__ */
