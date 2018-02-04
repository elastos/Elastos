#ifndef __TEST_CONFIG_H__
#define __TEST_CONFIG_H__

#include <limits.h>

#include "ela_carrier.h"

typedef struct TestConfig {
    int shuffle;
    int log2file;

    struct {
        int loglevel;
        char data_location[PATH_MAX];
    } tests;

    struct {
        int loglevel;
        char data_location[PATH_MAX];
    } robot;

    int bootstraps_size;
    BootstrapNode **bootstraps;
} TestConfig;

extern TestConfig global_config;

void load_config(const char *config_file);

#endif /* __TEST_CONFIG_H__ */
