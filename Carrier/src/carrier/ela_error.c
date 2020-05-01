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
#include <pthread.h>

#include <crystal.h>

#include "ela_carrier.h"
#include "ela_carrier_impl.h"

#if defined(_WIN32) || defined(_WIN64)
#define __thread        __declspec(thread)
#endif

#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
static __thread int ela_error;
#elif defined(__APPLE__)
#include <pthread.h>
static pthread_once_t ela_key_once = PTHREAD_ONCE_INIT;
static pthread_key_t ela_error;
static void ela_setup_error(void)
{
    (void)pthread_key_create(&ela_error, NULL);
}
#else
#error "Unsupported OS yet"
#endif

int ela_get_error(void)
{
#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
    return ela_error;
#elif defined(__APPLE__)
    return (int)pthread_getspecific(ela_error);
#else
#error "Unsupported OS yet"
#endif
}

void ela_clear_error(void)
{
#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
    ela_error = ELASUCCESS;
#elif defined(__APPLE__)
    (void)pthread_setspecific(ela_error, 0);
#else
#error "Unsupported OS yet"
#endif
}

void ela_set_error(int err)
{
#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
    ela_error = err;
#elif defined(__APPLE__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
    (void)pthread_once(&ela_key_once, ela_setup_error);
    (void)pthread_setspecific(ela_error, (void*)err);
#pragma GCC diagnostic pop
#else
#error "Unsupported OS yet"
#endif
}

typedef struct ErrorDesc {
    int errcode;
    const char *errdesc;
} ErrorDesc;

static
const ErrorDesc error_codes[] = {
    { ELAERR_INVALID_ARGS,                "Invalid argument(s)"     },
    { ELAERR_OUT_OF_MEMORY,               "Out of memory"           },
    { ELAERR_BUFFER_TOO_SMALL,            "Too small buffer size"   },
    { ELAERR_BAD_PERSISTENT_DATA,         "Bad persistent data"     },
    { ELAERR_INVALID_PERSISTENCE_FILE,    "Invalid persistent file" },
    { ELAERR_INVALID_CONTROL_PACKET,      "Invalid control packet"  },
    { ELAERR_INVALID_CREDENTIAL,          "Invalid credential"      },
    { ELAERR_ALREADY_RUN,                 "Carrier is already being running" },
    { ELAERR_NOT_READY,                   "Carrier is not ready"    },
    { ELAERR_NOT_EXIST,                   "Friend does not exist"   },
    { ELAERR_ALREADY_EXIST,               "Friend already exists"   },
    { ELAERR_NO_MATCHED_REQUEST,          "Unmatched request"       },
    { ELAERR_INVALID_USERID,              "Invalid carrier userid"  },
    { ELAERR_INVALID_NODEID,              "Invalid carrier nodeid"  },
    { ELAERR_WRONG_STATE,                 "Being in wrong state"    },
    { ELAERR_BUSY,                        "Instance is being busy"  },
    { ELAERR_LANGUAGE_BINDING,            "Language binding error"  },
    { ELAERR_ENCRYPT,                     "Encrypt error"           },
    { ELAERR_SDP_TOO_LONG,                "SDP is too long"         },
    { ELAERR_INVALID_SDP,                 "Invalid SDP"             },
    { ELAERR_NOT_IMPLEMENTED,             "Not implemented yet"     },
    { ELAERR_LIMIT_EXCEEDED,              "Exceeding the limit"     },
    { ELAERR_PORT_ALLOC,                  "Allocate port error"     },
    { ELAERR_BAD_PROXY_TYPE,              "Bad proxy type"          },
    { ELAERR_BAD_PROXY_HOST,              "Bad proxy host"          },
    { ELAERR_BAD_PROXY_PORT,              "Bad proxy port"          },
    { ELAERR_PROXY_NOT_AVAILABLE,         "No proxy available"      },
    { ELAERR_ENCRYPTED_PERSISTENT_DATA,   "Load encrypted persistent data error"},
    { ELAERR_BAD_BOOTSTRAP_HOST,          "Bad bootstrap host"      },
    { ELAERR_BAD_BOOTSTRAP_PORT,          "Bad bootstrap port"      },
    { ELAERR_TOO_LONG,                    "Data content too long"   },
    { ELAERR_ADD_SELF,                    "Try add myself as friend"},
    { ELAERR_BAD_ADDRESS,                 "Bad carrier node address"},
    { ELAERR_FRIEND_OFFLINE,              "Friend is being offline" },
    { ELAERR_UNKNOWN,                     "Unknown error"           }
};

static int general_error(int errcode, char *buf, size_t len)
{
    int size = sizeof(error_codes)/sizeof(ErrorDesc);
    int i;

    for (i = 0; i < size; i++) {
        if (errcode == error_codes[i].errcode)
            break;
    }

    if (i >= size || len <= strlen(error_codes[i].errdesc))
        return ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS);

    strcpy(buf, error_codes[i].errdesc);
    return 0;
}

static int system_error(int errcode, char *buf, size_t len)
{
    int rc;
#if defined(_WIN32) || defined(_WIN64)
    rc = strerror_s(buf, len, errcode);
#else
    rc = strerror_r(errcode, buf, len);        
#endif
    if (rc < 0)
        return ELA_SYS_ERROR(ELAERR_INVALID_ARGS);

    return 0;
}

static int dht_error(int errcode, char *buf, size_t len)
{
    int size = sizeof(error_codes)/sizeof(ErrorDesc);
    int i;

    for (i = 0; i < size; i++) {
        if (errcode == error_codes[i].errcode)
            break;
    }

    if (i >= size || len <= strlen(error_codes[i].errdesc))
        return ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS);

    strcpy(buf, error_codes[i].errdesc);
    return 0;
}

static int express_error(int errcode, char *buf, size_t len)
{
    int size = sizeof(error_codes)/sizeof(ErrorDesc);
    int i;

    for (i = 0; i < size; i++) {
        if (errcode == error_codes[i].errcode)
            break;
    }

    if (i >= size || len <= strlen(error_codes[i].errdesc))
        return ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS);

    strcpy(buf, error_codes[i].errdesc);
    return 0;
}

typedef struct FacilityDesc {
    const char *desc;
    strerror_t errstring;
} FacilityDesc;

static FacilityDesc facility_codes[] = {
    { "[General] ",         general_error },    //ELAF_GENERAL
    { "[System] ",          system_error },     //ELAF_SYS
    { "Reserved facility",  NULL },             //ELAF_RESERVED1
    { "Reserved facility",  NULL },             //ELAF_RESERVED2
    { "[ICE] ",             NULL },             //ELAF_ICE
    { "[DHT] ",             dht_error },        //ELAF_DHT
    { "[Express] ",         express_error },    //ELAF_EXPRESS
};

char *ela_get_strerror(int error, char *buf, size_t len)
{
    FacilityDesc *faci_desc;
    bool negative;
    int facility;
    int errcode;
    int rc = 0;
    size_t desc_len;
    char *p = buf;

    negative = !!(error & 0x80000000);
    facility = (error >> 24) & 0x0F;
    errcode  = error & 0x00FFFFFF;

    if (!buf || !negative || facility <= 0 ||
        facility > sizeof(facility_codes)/sizeof(FacilityDesc)) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return NULL;
    }

    faci_desc = (FacilityDesc*)&facility_codes[facility - 1];
    desc_len = strlen(faci_desc->desc);
    if (len < desc_len) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_BUFFER_TOO_SMALL));
        return NULL;
    }

    strcpy(p, faci_desc->desc);
    p += desc_len;
    len -= desc_len;

    if (faci_desc->errstring)
        rc = faci_desc->errstring(errcode, p, len);

    if (rc < 0) {
        ela_set_error(rc);
        return NULL;
    }

    return buf;
}

int ela_register_strerror(int facility, strerror_t user_strerr)
{
    FacilityDesc *faci_desc;

    if (facility <= 0 || facility > ELAF_DHT) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    faci_desc = (FacilityDesc*)&facility_codes[facility - 1];
    if (!faci_desc->errstring)
        faci_desc->errstring = user_strerr;

    return 0;
}
