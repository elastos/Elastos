#ifndef __ELACP_H__
#define __ELACP_H__

#include <stdint.h>
#include <stdbool.h>
#include "ela_carrier.h"

typedef struct ElaCP ElaCP;

/* WMCP types */
#define ELACP_TYPE_MIN                        1

#define ELACP_TYPE_USERINFO                   3

#define ELACP_TYPE_FRIEND_REQUEST             6
#define ELACP_TYPE_FRIEND_REMOVE              7

#define ELACP_TYPE_MESSAGE                    33
#define ELACP_TYPE_INVITE_REQUEST             34
#define ELACP_TYPE_INVITE_RESPONSE            35

#define ELACP_TYPE_MAX                        95

ElaCP *elacp_create(uint8_t type, const char *ext_name);

void elacp_free(ElaCP *cp);

int elacp_get_type(ElaCP *cp);

const char *elacp_get_extension(ElaCP *cp);

const char *elacp_get_name(ElaCP *cp);

const char *elacp_get_descr(ElaCP *cp);

bool elacp_get_has_avatar(ElaCP *cp);

const char *elacp_get_gender(ElaCP *cp);

const char *elacp_get_phone(ElaCP *cp);

const char *elacp_get_email(ElaCP *cp);

const char *elacp_get_region(ElaCP *cp);

const char *elacp_get_hello(ElaCP *cp);

int64_t elacp_get_tid(ElaCP *cp);

int elacp_get_status(ElaCP *cp);

const void *elacp_get_raw_data(ElaCP *cp);

size_t elacp_get_raw_data_length(ElaCP *cp);

const char *elacp_get_reason(ElaCP *cp);

void elacp_set_name(ElaCP *cp, const char *name);

void elacp_set_descr(ElaCP *cp, const char *descr);

void elacp_set_has_avatar(ElaCP *cp, int has_avatar);

void elacp_set_gender(ElaCP *cp, const char *gender);

void elacp_set_phone(ElaCP *cp, const char *phone);

void elacp_set_email(ElaCP *cp, const char *email);

void elacp_set_region(ElaCP *cp, const char *region);

void elacp_set_hello(ElaCP *cp, const char *hello);

void elacp_set_tid(ElaCP *cp, int64_t *tid);

void elacp_set_status(ElaCP *cp, int status);

void elacp_set_raw_data(ElaCP *cp, const void *data, size_t len);

void elacp_set_reason(ElaCP *cp, const char *reason);

uint8_t *elacp_encode(ElaCP *cp, size_t *len);

ElaCP *elacp_decode(const uint8_t *buf, size_t len);

#endif /* __ELACP_H__ */
