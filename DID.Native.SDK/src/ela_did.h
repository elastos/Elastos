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

#ifndef __ELA_DID_H__
#define __ELA_DID_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>

#if defined(__APPLE__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdocumentation"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(DID_STATIC)
  #define DID_API
#elif defined(DID_DYNAMIC)
  #ifdef DID_BUILD
    #if defined(_WIN32) || defined(_WIN64)
      #define DID_API         __declspec(dllexport)
    #else
      #define DID_API         __attribute__((visibility("default")))
    #endif
  #else
    #if defined(_WIN32) || defined(_WIN64)
      #define DID_API         __declspec(dllimport)
    #else
      #define DID_API         __attribute__((visibility("default")))
    #endif
  #endif
#else
  #define DID_API
#endif

typedef struct DID DID;
typedef struct DIDDocument DIDDocument;

typede int PublicKeyCallback(PublicKey *pk);

typedef struct DID {
  char* sepecific_idstring;
  char* id_fragment;
  DIDDocument document;
}DID;

typedef struct PublicKey{
  DID did;
  char* type;
  DID controller;
  char* PublicKeyBase;
}PublicKey;

//static PublicKey pbs[?];

typedef struct Authorization{
  PublicKeyElement pbs[?];
  //todo: need pk_size?
  int pk_size;
}Authorization;

//static Authorization auts[?];

typedef struct CredentialSubject{
  char* name;
  char* firstname;
  char* lastname;
  char* nickname;
  char* gender;
  char* nation;
  char* language;
  //todo: more types to inclare
} CredentialSubject;

typedef struct CredentialProof{
  char* type;
  char* verificationMethod;
  char* signatureValue;
}CredentialProof;

typedef struct VerCredential{
  DID id;
  char* type;
  char* issuanceData;
  CredentialSubject subject;
  CredentialProof proof;
}VerifiableCredential;

typedef struct Service{
  DID id;
  char* type;
  char* service_point;
}Service;

typedef struct Expires{
  char* expires;
}Expires;

//static service services[?];

typedef struct DIDDocument{
  DID did;
  PublicKey* pubulic_keys;
  Authorization* authorization;
  VerCredential* credential;
  Service* service;
  Expires* extime;
}DIDDocument;

DID_API void dummy(void);

DID_API int DIDDocument_Create();

//for DID
DID_API DID* DID_From_String(const char* idstring);

DID_API DID* DID_From_Json(const char *json);

DID_API DIDDocument* DID_Resolve(DID *did, in update);

DID_API const char* DID_Get_Method(DID* did);

DID_API const char* DID_Get_SpecificId(DID* did);

DID_API const char* DID_Get_DidString(DID* did);

DID_API void DID_Close(DID* did);

//for DIDDocumet
DID_API DID* DIDDocument_Get_Subject(DIDDocument *document);

DID_API size_t DIDDocument_Get_PublicKey_Count(DIDDocument *document);

DID_API PublicKey* DIDDocument_Get_PublicKey(DIDDocument *document, const char* id);

DID_API int DIDDocument_Get_PublicKey_Array(DIDDocument *document, PublicKeyCallback callback);

DID_API int DIDDocument_Select_PublicKey(DIDDocument *document, const char* id, const char* type, PublicKeyCallback callback);

DID_API void DIDDocument_Close(DIDDocument *document);

//DID_API int DIDDocument_Update();

//DID_API int DIDDocument_rename();

//DID_API int DIDDocument_Deactivate();



#endif /* __ELA_DID_H__ */