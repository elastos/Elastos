#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#ifdef HAVE_IO_H
#include <io.h>
#endif
#include <fcntl.h>
#include <crystal.h>

#include "loader.h"

static size_t len = 5000;

char *global_did_string;
char *global_didbp_string;
char *global_cred_string;

int load_file(const char *config_file, Load_Type type)
{
    int fd;
    size_t rec_len;

    if(!config_file)
        return -1;

    fd = open(config_file, O_RDONLY);
    if (fd == -1)
        return -1;

    if (type == Load_Doc) {
        global_did_string = calloc(1, len);
        if(!global_did_string)
            return -1;

        rec_len = read(fd, global_did_string, len);
        if(rec_len == 0 || rec_len == -1)
            return -1;
    }

    if (type == Load_Docbp) {
        global_didbp_string = calloc(1, len);
        if(!global_didbp_string)
            return -1;

        rec_len = read(fd, global_didbp_string, len);
        if(rec_len == 0 || rec_len == -1)
            return -1;
    }

    if (type == Load_Credential) {
        global_cred_string = calloc(1, len);
        if(!global_cred_string)
            return -1;

        rec_len = read(fd, global_cred_string, len);
        if(rec_len == 0 || rec_len == -1)
            return -1;
    }

    close(fd);
    return 0;
}