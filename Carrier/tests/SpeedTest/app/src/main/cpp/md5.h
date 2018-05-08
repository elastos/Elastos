//
// Created by kunshanyu on 20/04/2018.
//

#ifndef SPEEDTEST_MD5_H
#define SPEEDTEST_MD5_H

#ifdef __cplusplus
extern "C"
{
#endif                          /* __cplusplus */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#ifdef HAVE_STDINT_H
#include <stdint.h>
typedef uint32_t uint32;
#else
// A.Leo.: this wont work on 16 bits platforms ;)
typedef unsigned uint32;
#endif


//#define MD5_FILE_BUFFER_LEN 1024

struct MD5Context {
    uint32 buf[4];
    uint32 bits[2];
    unsigned char in[64];
};

typedef struct MD5Context MD5_CTX;
/*
void MD5Init(struct MD5Context *context);
void MD5Update(struct MD5Context *context, unsigned char const *buf,
               unsigned len);
void MD5Final(unsigned char digest[16], struct MD5Context *context);
void MD5Transform(uint32 buf[4], uint32 const in[16]);
*/

//int getBytesMD5(const unsigned char* src, unsigned int length, char* md5);
//int getStringMD5(const char* src, char* md5);
int init_md5(MD5_CTX *context);
void update_md5(MD5_CTX *context, char *src, unsigned int len);
int get_bytes_md5(MD5_CTX *context, char *buf, int len);
int get_file_md5(const char *path, char *buf, int len);

/*
 * This is needed to make RSAREF happy on some MS-DOS compilers.
 */

#ifdef __cplusplus
}
#endif                          /* __cplusplus */

#endif //SPEEDTEST_MD5_H
