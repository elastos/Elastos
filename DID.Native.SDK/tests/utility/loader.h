/*
 * Copyright (c) 2019 Elastos Foundation
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

#ifndef __TEST_LOADER_H__
#define __TEST_LOADER_H__

#include "ela_did.h"
#include "HDkey.h"

#ifdef __cplusplus
extern "C" {
#endif

const char *get_store_path(char* path, const char *dir);

char *get_path(char *path, const char *file);

char *get_file_path(char *path, size_t size, int count, ...);

char *get_wallet_path(char* path, const char* dir);

bool file_exist(const char *path);

bool dir_exist(const char* path);

void delete_file(const char *path);

const char *Generater_Publickey(char *publickeybase58, size_t size);

DerivedKey *Generater_KeyPair(DerivedKey *dkey);

int Set_Doc_Txid(DIDDocument *doc, const char *txid);

int Set_Doc_Signature(DIDDocument *doc, const char *signature);

DID *DID_Copy(DID *dest, DID *src);

DIDURL *DIDURL_Copy(DIDURL *dest, DIDURL *src);

////////////////////////////////////////
void TestData_Init(void);

void TestData_Deinit(void);

DIDAdapter *TestData_GetAdapter(bool dummybackend);

DIDStore *TestData_SetupStore(bool dummybackend, const char *root);

void TestData_Free(void);

int TestData_InitIdentity(DIDStore *store);

const char *TestData_LoadIssuerJson(void);

const char *TestData_LoadIssuerCompJson(void);

const char *TestData_LoadIssuerNormJson(void);

const char *TestData_LoadDocJson(void);

const char *TestData_LoadDocCompJson(void);

const char *TestData_LoadDocNormJson(void);

Credential *TestData_LoadProfileVc(void);

const char *TestData_LoadProfileVcCompJson(void);

const char *TestData_LoadProfileVcNormJson(void);

Credential *TestData_LoadEmailVc(void);

const char *TestData_LoadEmailVcCompJson(void);

const char *TestData_LoadEmailVcNormJson(void);

Credential *TestData_LoadPassportVc(void);

const char *TestData_LoadPassportVcCompJson(void);

const char *TestData_LoadPassportVcNormJson(void);

Credential *TestData_LoadTwitterVc(void);

const char *TestData_LoadTwitterVcCompJson(void);

const char *TestData_LoadTwitterVcNormJson(void);

Credential *TestData_LoadVc(void);

const char *TestData_LoadVcCompJson(void);

const char *TestData_LoadVcNormJson(void);

Presentation *TestData_LoadVp(void);

const char *TestData_LoadVpNormJson(void);

DIDDocument *TestData_LoadDoc(void);

DIDDocument *TestData_LoadIssuerDoc(void);

const char *TestData_LoadRestoreMnemonic(void);

#ifdef __cplusplus
}
#endif

#endif /* __TEST_LOADER_H__ */
