#ifndef __TEST_LOADER_H__
#define __TEST_LOADER_H__

extern char *global_did_string;
extern char *global_didbp_string;
extern char *global_cred_string;

typedef enum Load_Type {
    Load_Doc,
    Load_Docbp,
    Load_Credential
} Load_Type;

int load_file(const char *config_file, Load_Type type);

#endif /* __TEST_LOADER_H__ */
