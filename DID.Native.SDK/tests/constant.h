#ifndef __CONTANTS_H__
#define __CONTANTS_H__

extern const char *walletId;
extern const char *network;
extern const char *resolver;
extern const char *storepass;

extern const char *type;
extern const char *service_type;
extern const char *mnemonic;

char *get_store_path(char* path, const char* dir);

char *get_wallet_path(char* path, const char* dir);


#endif /* __CONTANTS_H__ */
