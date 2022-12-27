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

#ifndef __ELA_DID_H__
#define __ELA_DID_H__

#include <stdbool.h>
#include <stdarg.h>
#include <sys/types.h>
#include <stdint.h>

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

#if defined(_WIN32) || defined(_WIN64)
typedef ptrdiff_t       ssize_t;
#endif

/**
 * \~English
 * DID string max length. eg, did:elastos:ixxxxxxxxxx
 */
#define ELA_MAX_DID_LEN                 128
/**
 * \~English
 * DIDURL string max length. eg, did:elastos:ixxxxxxxxxx#xxxxxx
 */
#define ELA_MAX_DIDURL_LEN              256
/**
 * \~English
 * DIDDocument and Credential alias max length.
 */
#define ELA_MAX_ALIAS_LEN               64
/**
 * \~English
 * DID transaction id max length.
 */
#define ELA_MAX_TXID_LEN                128
/**
 * \~English
 * Mnemonic max length.
 */
#define ELA_MAX_MNEMONIC_LEN            128

/**
 * \~English
 * Indicate the DID type to list.
 */
typedef enum {
    /**
     * \~English
     * List all dids.
     */
    DIDFilter_All = 0,
    /**
     * \~English
     * List dids that contain private key.
     */
    DIDFilter_HasPrivateKey = 1,
    /**
     * \~English
     * List dids without private key contained.
     */
    DIDFilter_WithoutPrivateKey = 2
} ELA_DID_FILTER;

/**
 * \~English
 * The value of the credential Subject property is defined as
 * a set of objects that contain one or more properties that are
 * each related to a subject of the credential.
 */
typedef struct Property {
    /**
     * \~English
     * Property key.
     */
    char *key;
    /**
     * \~English
     * Property value.
     */
    char *value;
} Property;

/**
 * \~English
 * DID is a globally unique identifier that does not require
 * a centralized registration authority.
 * It includes method specific string. (elastos:id:ixxxxxxxxxx).
 */
typedef struct DID                  DID;
/**
 * \~English
 * DID URL defines by the did-url rule, refers to a URL that begins with a DID
 * followed by one or more additional components. A DID URL always
 * identifies the resource to be located.
 * DIDURL includes DID and Url fragment by user defined.
 */
typedef struct DIDURL               DIDURL;
/**
 * \~English
 * Public keys are used for digital signatures, encryption and
 * other cryptographic operations, which are the basis for purposes such as
 * authentication or establishing secure communication with service endpoints.
 */
typedef struct PublicKey            PublicKey;
/**
 * \~English
 * Credential is a set of one or more claims made by the same entity.
 * Credentials might also include an identifier and metadata to
 * describe properties of the credential.
 */
typedef struct Credential           Credential;
/**
 * \~English
 * CredentialMetaData stores information about Credential except information in Credential.
 */
typedef struct CredentialMetaData   CredentialMetaData;
/**
 * \~English
 * A Presentation can be targeted to a specific verifier by using a Linked Data
 * Proof that includes a nonce and realm.
 * This also helps prevent a verifier from reusing a verifiable presentation as
 * their own.
 */
typedef struct Presentation         Presentation;
/**
 * \~English
 * A service endpoint may represent any type of service the subject
 * wishes to advertise, including decentralized identity management services
 * for further discovery, authentication, authorization, or interaction.
 */
typedef struct Service              Service;
/**
 * \~English
 * A DID resolves to a DID Document. This is the concrete serialization of
 * the data model, according to a particular syntax.
 * DIDDocument is a set of data that describes the subject of a DID,
 * including public key, authentication(optional), authorization(optional),
 * credential and services. One document must be have only subject,
 * and at least one public key.
 */
typedef struct DIDDocument          DIDDocument;
/**
 * \~English
 DIDMetaData is store for other information about DID except DIDDocument information.
 */
typedef struct DIDMetaData          DIDMetaData;
/**
 * \~English
 DIDHistroy stores all did transactions from chain.
 */
typedef struct DIDHistory           DIDHistory;
/**
 * \~English
 * A DIDDocument Builder to modify DIDDocument elems.
 */
typedef struct DIDDocumentBuilder   DIDDocumentBuilder;
/**
 * \~English
 * A issuer is the did to issue credential. Issuer includes issuer's did and
 * issuer's sign key.
 */
typedef struct Issuer               Issuer;
/**
 * \~English
 * DIDStore is local store for specified DID.
 */
typedef struct DIDStore             DIDStore;
/**
 * \~English
 * DIDAdapter is support method to create did transaction.
 */
typedef struct DIDAdapter           DIDAdapter;
/**
 * \~English
 * DIDResolver is support method to resolve did document from chain.
 */
typedef struct DIDResolver          DIDResolver;
/**
 * \~English
 * JWTBuilder records the content about jwt.
 */
typedef struct JWTBuilder           JWTBuilder;
/**
 * \~English
 * DID list callbacks, return alias about did.
 * @param
 *      did               [in] A handle to DID.
 * @param
 *      context           [in] The application defined context data.
 * @return
 *      If no error occurs, return 0. Otherwise, return -1.
 */
typedef int DIDStore_DIDsCallback(DID *did, void *context);
/**
 * \~English
 * Credential list callbacks, return alias about credential.
 * @param
 *      did               [in] A handle to DID.
 * @param
 *      context           [in] The application defined context data.
 * @return
 *      If no error occurs, return 0. Otherwise, return -1.
 */
typedef int DIDStore_CredentialsCallback(DIDURL *id, void *context);
/**
 * \~English
 * The function indicate how to resolve the confict, if the local document is different
 * with the one resolved from chain.
 * @param
 *      chaincopy           [in] The document from DIDStore.
 * @param
 *      localcopy           [in] The document from chain.
 * @return
 *      If no error occurs, return merged document. Otherwise, return NULL.
 */
typedef DIDDocument* DIDStore_MergeCallback(DIDDocument *chaincopy, DIDDocument *localcopy);
/**
 * \~English
 * The function indicate how to get local did document, if this did is not published to chain.
 * @param
 *      did                 [in] The DID string.
 * @return
 *      If no error occurs, return the handle to DIDDocument. Otherwise, return NULL.
 */
typedef DIDDocument* DIDLocalResovleHandle(DID *did);
/**
 * \~English
 * DIDAdapter is support method to create did transaction.
 */
struct DIDAdapter {
/**
 * \~English
 * User need to implement 'createIdTransaction' function.
 * An application-defined function that create id transaction to chain.
 * @param
 *      adapter              [in] A handle to DIDAdapter.
 * @param
 *      payload              [in] The content of id transaction to publish.
 * @param
 *      memo                 [in] Memo string.
 * @return
 *      If no error occurs, return true. Otherwise, return false.
 */
    bool (*createIdTransaction) (DIDAdapter *adapter,
            const char *payload, const char *memo);
};
/**
 * \~English
 * DIDResolver is support method to resolve did document from chain.
 * User need to implement 'resolve' function.
 */
struct DIDResolver {
    /**
     * \~English
     * User need to implement 'createIdTransaction' function.
     * An application-defined function that resolve data from chain.
     * @param
     *      resolver             [in] A handle to DIDResolver.
     * @param
     *      did                  [in] Specified DID.
     * @param
     *      all                  [in] Resolve all transaction data or the lastest one.
     *                           all = 1: all transaction; all = 0: only the lastest transaction.
     * @return
     *      If no error occurs, return transaction id.
     *      Otherwise, return NULL.
     */
    const char* (*resolve) (DIDResolver *resolver, const char *did, int all);
};

/******************************************************************************
 * Log configuration.
 *****************************************************************************/
/**
 * \~English
 * DID log level to control or filter log output.
 */
typedef enum DIDLogLevel {
    /**
     * \~English
     * Log level None
     * Indicate disable log output.
     */
    DIDLogLevel_None = 0,
    /**
     * \~English
     * Log level fatal.
     * Indicate output log with level 'Fatal' only.
     */
    DIDLogLevel_Fatal = 1,
    /**
     * \~English
     * Log level error.
     * Indicate output log above 'Error' level.
     */
    DIDLogLevel_Error = 2,
    /**
     * \~English
     * Log level warning.
     * Indicate output log above 'Warning' level.
     */
    DIDLogLevel_Warning = 3,
    /**
     * \~English
     * Log level info.
     * Indicate output log above 'Info' level.
     */
    DIDLogLevel_Info = 4,
    /**
     * \~English
     * Log level debug.
     * Indicate output log above 'Debug' level.
     */
    DIDLogLevel_Debug = 5,
    /**
     * \~English
     * Log level trace.
     * Indicate output log above 'Trace' level.
     */
    DIDLogLevel_Trace = 6,
    /**
     * \~English
     * Log level verbose.
     * Indicate output log above 'Verbose' level.
     */
    DIDLogLevel_Verbose = 7
} DIDLogLevel;

/******************************************************************************
 * DID
 *****************************************************************************/
/**
 * \~English
 * Get DID from string.
 *
 * @param
 *      idstring     [in] A pointer to string including id information.
 *                        idstring support:   did:elastos:ixxxxxxx
 * @return
 *      If no error occurs, return the pointer of DID.
 *      Otherwise, return NULL.
 *      Notice that user need to release the handle of returned instance to destroy it's memory.
 */
DID_API DID *DID_FromString(const char *idstring);

/**
 * \~English
 * Create a new DID according to method specific string.
 *
 * @param
 *      method_specific_string    [in] A pointer to specific string.
 *                                     The method-specific-id value should be
 *                                     globally unique by itself.
 * @return
 *      If no error occurs, return the pointer of DID.
 *      Otherwise, return NULL, and a specific error code can be
 *      retrieved by calling ela_get_error().
 *      Notice that user need to release the handle of returned instance to destroy it's memory.
 */
DID_API DID *DID_New(const char *method_specific_string);

/**
 * \~English
 * Get method of DID.
 *
 * @param
 *      did                 [in] A handle to DID.
 * @return
 *      If no error occurs, return method string.
 *      Otherwise, return NULL, and a specific error code can be
 *      retrieved by calling ela_get_error().
 */
DID_API const char *DID_GetMethod(DID *did);

/**
 * \~English
 * Get method specific string of DID.
 *
 * @param
 *      did                  [in] A handle to DID.
 * @return
 *      If no error occurs, return string.
 *      Otherwise, return NULL, and a specific error code can be
 *      retrieved by calling ela_get_error().
 */
DID_API const char *DID_GetMethodSpecificId(DID *did);

/**
 * \~English
 * Get id string from DID.
 *
 * @param
 *      did                  [in] A handle to DID.
 * @param
 *      idstring             [out] The buffer that will receive the id string.
 *                                 The buffer size should at least (ELA_MAX_DID_LEN) bytes.
 * @param
 *      len                  [in] The buffer size of idstring.
 * @return
 *      The id string pointer, or NULL if buffer is too small.
 */
DID_API char *DID_ToString(DID *did, char *idstring, size_t len);

/**
 * \~English
 * Compare two DID is same or not.
 *
 * @param
 *      did1                  [in] One DID to be compared.
 * @param
 *      did2                  [in] The other DID to be compared.
 * @return
 *      true if two DID are same, or false if not.
 */
DID_API bool DID_Equals(DID *did1, DID *did2);

/**
 * \~English
 * Compare two DIDs with their did string.
 *
 * @param
 *      did1                   [in] One DID to be compared.
 * @param
 *      did2                   [in] The other DID to be compared.
 * @return
 *      return value < 0, it indicates did1 is less than did2.
 *      return value = 0, it indicates did1 is equal to did2.
 *      return value > 0, it indicates did1 is greater than did2.
 */
DID_API int DID_Compare(DID *did1, DID *did2);

/**
 * \~English
 * Destroy DID.
 *
 * @param
 *      did                   [in] A handle to DID to be destroied.
 */
DID_API void DID_Destroy(DID *did);

/**
 * \~English
 * Get the newest DID Document from chain.
 *
 * @param
 *      did                      [in] The handle of DID.
 * @param
 *      force                    [in] Indicate if load document from cache or not.
 *                               force = true, document gets only from chain.
 *                               force = false, document can get from cache,
 *                               if no document is in the cache, resolve it from chain.
 * @return
 *      If no error occurs, return the handle to DID Document.
 *      Otherwise, return NULL.
 *      Notice that user need to release the handle of returned instance to destroy it's memory.
 */
DID_API DIDDocument *DID_Resolve(DID *did, bool force);

/**
 * \~English
 * Get all DID Documents from chain.
 *
 * @param
 *      did                      [in] The handle of DID.
 * @return
 *      when no error occurs, it returns the handle to DIDHistory instance.
 *      otherwise, it returns NULL.
 *      Notice that user need to release the handle of returned instance to destroy it's memory.
 */
DID_API DIDHistory *DID_ResolveHistory(DID *did);

/**
 * \~English
 * Get DID MetaData from did.
 *
 * @param
 *      did                      [in] The handle of DID.
 * @return
 *      If no error occurs, return the handle to DIDMetaData.
 *      Otherwise, return -1.
 */
DID_API DIDMetaData *DID_GetMetaData(DID *did);

/**
 * \~English
 * Save DID MetaData.
 *
 * @param
 *      did                      [in] The handle of DID.
 * @return
 *      If no error occurs, return 0.
 *      Otherwise, return -1.
 */
DID_API int DID_SaveMetaData(DID *did);

/**
 * \~English
 * Get alias for did.
 *
 * @param
 *      metadata                        [in] The handle of DIDMetaData.
 * @return
 *      If no error occurs, return alias string.
 *      Otherwise, return NULL.
 */
DID_API const char *DIDMetaData_GetAlias(DIDMetaData *metadata);

/**
 * \~English
 * Get did status, deactived or not.
 *
 * @param
 *      metadata                        [in] The handle of DIDMetaData.
 * @return
 *      If no error occurs, return status.
 *      Otherwise, return false.
 */
DID_API bool DIDMetaData_GetDeactivated(DIDMetaData *metadata);

/**
 * \~English
 * Get the time of transaction id for did.
 *
 * @param
 *      metadata                        [in] The handle of DIDMetaData.
 * @return
 *      If no error occurs, return time stamp.
 *      Otherwise, return 0.
 */
DID_API time_t DIDMetaData_GetPublished(DIDMetaData *metadata);

/**
 * \~English
 * Set alias for did.
 *
 * @param
 *      metadata                        [in] The handle of DIDMetaData.
 * @param
 *      alias                           [in] The ailas string.
 * @return
 *      If no error occurs, return 0. Otherwise, return -1.
 */
DID_API int DIDMetaData_SetAlias(DIDMetaData *metadata, const char *alias);

/**
 * \~English
 * Set 'string' extra elemfor did.
 *
 * @param
 *      metadata                        [in] The handle of DIDMetaData.
 * @param
 *      key                             [in] The key string.
 * @param
 *      value                           [in] The value string.
 * @return
 *      If no error occurs, return 0. Otherwise, return -1.
 */
DID_API int DIDMetaData_SetExtra(DIDMetaData *metadata, const char* key, const char *value);

/**
 * \~English
 * Set 'boolean' extra elem for did.
 *
 * @param
 *      metadata                        [in] The handle of DIDMetaData.
 * @param
 *      key                             [in] The key string.
 * @param
 *      value                           [in] The boolean value.
 * @return
 *      If no error occurs, return 0. Otherwise, return -1.
 */
DID_API int DIDMetaData_SetExtraWithBoolean(DIDMetaData *metadata, const char *key, bool value);

/**
 * \~English
 * Set 'double' extra elem for did.
 *
 * @param
 *      metadata                        [in] The handle of DIDMetaData.
 * @param
 *      key                             [in] The key string.
 * @param
 *      value                           [in] The double value.
 * @return
 *      If no error occurs, return 0. Otherwise, return -1.
 */
DID_API int DIDMetaData_SetExtraWithDouble(DIDMetaData *metadata, const char *key, double value);

/**
 * \~English
 * Get 'string' extra elem from DID.
 *
 * @param
 *      metadata                       [in] The handle of CredentialMetaData.
 * @param
 *      key                            [in] The key string.
 * @return
 *      If no error occurs, return the elem string. Otherwise, return NULL.
 */
DID_API const char *DIDMetaData_GetExtra(DIDMetaData *metadata, const char *key);

/**
 * \~English
 * Get 'boolean' extra elem from DID.
 *
 * @param
 *      metadata                       [in] The handle of CredentialMetaData.
 * @param
 *      key                            [in] The key string.
 * @return
 *      'boolean' elem value.
 */
DID_API bool DIDMetaData_GetExtraAsBoolean(DIDMetaData *metadata, const char *key);

/**
 * \~English
 * Get 'double' extra elem from DID.
 *
 * @param
 *      metadata                       [in] The handle of CredentialMetaData.
 * @param
 *      key                            [in] The key string.
 * @return
 *      'double' elem value.
 */
DID_API double DIDMetaData_GetExtraAsDouble(DIDMetaData *metadata, const char *key);

/******************************************************************************
 * DIDURL
 *****************************************************************************/
/**
 * \~English
 * Get DID URL from string.
 *
 * @param
 *      idstring     [in] A pointer to string including id information.
 *                   idstring support: 1. "did:elastos:xxxxxxx#xxxxx"
 *                                     2. "#xxxxxxx"
 * @param
 *      ref          [in] A pointer to DID.
 * @return
 *      If no error occurs, return the handle to DID URL.
 *      Otherwise, return NULL.
 *      Notice that user need to release the handle of returned instance to destroy it's memory.
 */
DID_API DIDURL *DIDURL_FromString(const char *idstring, DID *ref);

/**
 * \~English
 * Create a new DID URL according to specific string and fragment.
 *
 * @param
 *      method_specific_string    [in] A pointer to specific string.
 *                                     The method-specific-id value should be
 *                                     globally unique by itself.
 * @param
 *      fragment                  [in] The portion of a DID URL.
 * @return
 *      If no error occurs, return the handle to DID URL.
 *      Otherwise, return NULL.
 *      Notice that user need to release the handle of returned instance to destroy it's memory.
 */
DID_API DIDURL *DIDURL_New(const char *method_specific_string, const char *fragment);

/**
 * \~English
 * Create a new DID URL according to DID and fragment.
 *
 * @param
 *      did                       [in] A pointer to DID.
 * @param
 *      fragment                  [in] The portion of a DID URL.
 * @return
 *      If no error occurs, return the handle to DID URL.
 *      Otherwise, return NULL.
 *      Notice that user need to release the handle of returned instance to destroy it's memory.
 */
DID_API DIDURL *DIDURL_NewByDid(DID *did, const char *fragment);

/**
 * \~English
 * Get DID from DID URL.
 *
 * @param
 *      id               [in] A handle to DID URL.
 * @return
 *      If no error occurs, return the handle to DID.
 *      Otherwise, return NULL.
 */
DID_API DID *DIDURL_GetDid(DIDURL *id);

/**
 * \~English
 * Get fragment from DID URL.
 *
 * @param
 *      id               [in] A handle to DID URL.
 * @return
 *      If no error occurs, return fragment string.
 *      Otherwise, return NULL.
 */
DID_API const char *DIDURL_GetFragment(DIDURL *id);

/**
 * \~English
 * Get id string from DID URL.
 *
 * @param
 *      id               [in] A handle to DID URL.
 * @param
 *      idstring         [out] The buffer that will receive the id string.
 *                             The buffer size should at least (ELA_MAX_DID_LEN) bytes.
 * @param
 *      len              [in] The buffer size of idstring.
 * @param
 *      compact          [in] Id string is compact or not.
 *                       true represents compact, flase represents not compact.
 * @return
 *      If no error occurs, return id string. Otherwise, return NULL.
 */
DID_API char *DIDURL_ToString(DIDURL *id, char *idstring, size_t len, bool compact);

/**
 * \~English
 * Compare two DID URL is same or not.
 *
 * @param
 *      id1                  [in] One DID URL to be compared.
 * @param
 *      id2                  [in] The other DID URL to be compared.
 * @return
 *      true if two DID URL are same, or false if not.
 */
DID_API bool DIDURL_Equals(DIDURL *id1, DIDURL *id2);

/**
 * \~English
 * Compare two DIDURLs with their whole string.
 *
 * @param
 *      id1                   [in] One DID URL to be compared.
 * @param
 *      id2                   [in] The other DID URL to be compared.
 * @return
 *      return value < 0, it indicates id1 is less than id2.
 *      return value = 0, it indicates id1 is equal to id2.
 *      return value > 0, it indicates id1 is greater than id2.
 */
DID_API int DIDURL_Compare(DIDURL *id1, DIDURL *id2);

/**
 * \~English
 * Destroy DID URL.
 *
 * @param
 *      id                  [in] A handle to DID URL to be destroied.
 */
DID_API void DIDURL_Destroy(DIDURL *id);

/**
 * \~English
 * Get CredentialMetaData from Credential.
 *
 * @param
 *      id                       [in] The handle of DIDURL.
 * @return
 *      If no error occurs, return the handle to CredentialMetaData. Otherwise, return NULL.
 */
DID_API CredentialMetaData *DIDURL_GetMetaData(DIDURL *id);

/**
 * \~English
 * Save Credential(DIDURL) MetaData.
 *
 * @param
 *      id                      [in] The handle of DIDURL.
 * @return
 *      If no error occurs, return 0.
 *      Otherwise, return -1.
 */
DID_API int DIDURL_SaveMetaData(DIDURL *id);

/**
 * \~English
 * Set alias for Credential.
 *
 * @param
 *      metadata                       [in] The handle of CredentialMetaData.
 * @param
 *      alias                          [in] The alias string.
 * @return
 *      If no error occurs, return the 0. Otherwise, return -1.
 */
DID_API int CredentialMetaData_SetAlias(CredentialMetaData *metadata, const char *alias);

/**
 * \~English
 * Set 'string' extra elem for Credential.
 *
 * @param
 *      metadata                       [in] The handle of CredentialMetaData.
 * @param
 *      key                            [in] The key string.
 * @param
 *      value                          [in] The value string.
 * @return
 *      If no error occurs, return the 0. Otherwise, return -1.
 */
DID_API int CredentialMetaData_SetExtra(CredentialMetaData *metadata,
        const char* key, const char *value);

/**
 * \~English
 * Set 'boolean' extra elem for Credential.
 *
 * @param
 *      metadata                       [in] The handle of CredentialMetaData.
 * @param
 *      key                            [in] The key string.
 * @param
 *      value                          [in] The boolean value.
 * @return
 *      If no error occurs, return the 0. Otherwise, return -1.
 */
DID_API int CredentialMetaData_SetExtraWithBoolean(CredentialMetaData *metadata,
        const char *key, bool value);

/**
 * \~English
 * Set 'double' extra elem for Credential.
 *
 * @param
 *      metadata                       [in] The handle of CredentialMetaData.
 * @param
 *      key                            [in] The key string.
 * @param
 *      value                          [in] The double value.
 * @return
 *      If no error occurs, return the 0. Otherwise, return -1.
 */
DID_API int CredentialMetaData_SetExtraWithDouble(CredentialMetaData *metadata,
        const char *key, double value);

/**
 * \~English
 * Get alias for id, mainly for credential.
 *
 * @param
 *      id                        [in] The handle of DIDURL.
 * @return
 *      If no error occurs, return alias string. Otherwise, return NULL.
 */
DID_API const char *CredentialMetaData_GetAlias(CredentialMetaData *metadata);

/**
 * \~English
 * Get 'string' extra elem from Credential.
 *
 * @param
 *      metadata                       [in] The handle of CredentialMetaData.
 * @param
 *      key                            [in] The key string.
 * @return
 *      If no error occurs, return the elem string. Otherwise, return NULL.
 */
DID_API const char *CredentialMetaData_GetExtra(CredentialMetaData *metadata,
        const char *key);

/**
 * \~English
 * Get 'boolean' extra elem from Credential.
 *
 * @param
 *      metadata                       [in] The handle of CredentialMetaData.
 * @param
 *      key                            [in] The key string.
 * @return
 *      'boolean' elem value.
 */
DID_API bool CredentialMetaData_GetExtraAsBoolean(CredentialMetaData *metadata,
        const char *key);

/**
 * \~English
 * Get 'double' extra elem from Credential.
 *
 * @param
 *      metadata                       [in] The handle of CredentialMetaData.
 * @param
 *      key                            [in] The key string.
 * @return
 *      'double' elem value.
 */
DID_API double CredentialMetaData_GetExtraAsDouble(CredentialMetaData *metadata,
        const char *key);

/******************************************************************************
 * DIDHistory
 *****************************************************************************/

/**
 * \~English
 * Get owner of DID resolved history.
 *
 * @param
 *      history                       [in] The handle to DIDHistory.
 * @return
 *      If no error occurs, return the handle to DID. Destroy DID after finishing use.
 *      Otherwise, return NULL.
 */
DID_API DID *DIDHistory_GetOwner(DIDHistory *history);

/**
 * \~English
 * Get DID status of DID.
 *
 * @param
 *      history                       [in] The handle to DIDHistory.
 * @return
*      If no error occurs, return DID status. Otherwise, return -1.
 */
DID_API int DIDHistory_GetStatus(DIDHistory *history);

/**
 * \~English
 * Get DID transaction count.
 *
 * @param
 *      history                       [in] The handle to DIDHistory.
 * @return
*      If no error occurs, return count. Otherwise, return -1.
 */
DID_API ssize_t DIDHistory_GetTransactionCount(DIDHistory *history);

/**
 * \~English
 * Get DID Document from 'index' transaction.
 *
 * @param
 *      history                       [in] The handle to DIDHistory.
 * @param
 *      idex                          [in] The index of transaction.
 * @return
 *      If no error occurs, return the handle to DID Document.
 *      Otherwise, return NULL.
 *      Notice that user need to release the handle of returned instance to destroy it's memory.
 */
DID_API DIDDocument *DIDHistory_GetDocumentByIndex(DIDHistory *history, int index);

/**
 * \~English
 * Get transaction id from 'index' transaction.
 *
 * @param
 *      history                       [in] The handle to DIDHistory.
 * @param
 *      idex                          [in] The index of transaction.
 * @return
 *      If no error occurs, return transaction.
 *      Otherwise, return NULL.
 */
DID_API const char *DIDHistory_GetTransactionIdByIndex(DIDHistory *history, int index);

/**
 * \~English
 * Get published time from 'index' transaction.
 *
 * @param
 *      history                       [in] The handle to DIDHistory.
 * @param
 *      idex                          [in] The index of transaction.
 * @return
*      If no error occurs, return published time. Otherwise, return 0.
 */
DID_API time_t DIDHistory_GetPublishedByIndex(DIDHistory *history, int index);

/**
 * \~English
 * Get operation of 'index' transaction. Operation: 'created', 'update' and 'deactivated'.
 *
 * @param
 *      history                       [in] The handle to DIDHistory.
 * @param
 *      idex                          [in] The index of transaction.
 * @return
 *       If no error occurs, return operation string.
 *       Otherwise, return -1.
 */
DID_API const char *DIDHistory_GetOperationByIndex(DIDHistory *history, int index);

/**
 * \~English
 * Destroy DIDHistory.
 *
 * @param
 *      history               [in] A handle to DIDHistory.
 */
DID_API void DIDHistory_Destroy(DIDHistory *history);

/******************************************************************************
 * DIDDocument
 *****************************************************************************/
/**
 * \~English
 * Get DID Document from json context.
 *
 * @param
 *      json               [in] Context of did conforming to json informat.
 * @return
 *      If no error occurs, return the handle to DID Document.
 *      Otherwise, return NULL.
 *      Notice that user need to release the handle of returned instance to destroy it's memory.
 */
DID_API DIDDocument *DIDDocument_FromJson(const char* json);

/**
 * \~English
 * Get json non-formatted context from DID Document.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      normalized           [in] Json context is normalized or not.
 *                           true represents normalized, false represents not compact.
 * @return
 *      If no error occurs, return json context. Otherwise, return NULL.
 *      Notice that user need to free the returned value that it's memory.
 */
DID_API const char *DIDDocument_ToJson(DIDDocument *document, bool normalized);


/**
 * \~English
 * Get json formatted context from DID Document.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      normalized           [in] Json context is normalized or not.
 *                           true represents normalized, false represents not compact.
 * @return
 *      If no error occurs, return json context. Otherwise, return NULL.
 *      Notice that user need to free the returned value that it's memory.
 */
DID_API const char *DIDDocument_ToString(DIDDocument *document, bool normalized);
/**
 * \~English
 * Destroy DID Document.
 *
 * @param
 *      document             [in] A handle to DID Document to be destroied.
 */
DID_API void DIDDocument_Destroy(DIDDocument *document);

/**
 * \~English
 * Check that document is deactivated or not.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @return
 *      true if document is deactivated, otherwise false.
*/
DID_API bool DIDDocument_IsDeactivated(DIDDocument *document);

/**
 * \~English
 * Check that document is genuine or not.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @return
 *      true if document is genuine, otherwise false.
*/
DID_API bool DIDDocument_IsGenuine(DIDDocument *document);

/**
 * \~English
 * Check that document is expired or not.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @return
 *      true if document is expired, otherwise false.
*/
DID_API bool DIDDocument_IsExpires(DIDDocument *document);

/**
 * \~English
 * Check that document is valid or not.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @return
 *      true if document is valid, otherwise false.
*/
DID_API bool DIDDocument_IsValid(DIDDocument *document);

/**
 * \~English
 * Get DID subject to DID Document. The DID Subject is the entity of
 * the DID Document. A DID Document must have exactly one DID subject.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @return
 *      If no error occurs, return a handle to DID.
 *      Otherwise, return NULL.
 */
DID_API DID* DIDDocument_GetSubject(DIDDocument *document);

/**
 * \~English
 * Get DIDDocument Builder to modify document.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @return
 *      If no error occurs, return a handle to DIDDocumentBuilder.
 *      Otherwise, return NULL.
 *      Notice that user need to release the handle of returned instance to destroy it's memory.
 */
DID_API DIDDocumentBuilder* DIDDocument_Edit(DIDDocument *document);

/**
 * \~English
 * Destroy DIDDocument Builder.
 *
 * @param
 *      builder             [in] A handle to DIDDocument Builder.
 */
DID_API void DIDDocumentBuilder_Destroy(DIDDocumentBuilder *builder);

/**
 * \~English
 * Finish modiy document.
 *
 * @param
 *      builder              [in] A handle to DIDDocument Builder.
 * @param
 *      storepass            [in] Pass word to sign.
 * @return
 *      If no error occurs, return a handle to DIDDocument.
 *      Otherwise, return NULL.
 *      Notice that user need to release the handle of returned instance to destroy it's memory.
 */
DID_API DIDDocument *DIDDocumentBuilder_Seal(DIDDocumentBuilder *builder,
            const char *storepass);

/**
 * \~English
 * Add public key to DID Document.
 * Each public key has an identifier (id) of its own, a type, and a controller,
 * as well as other properties publicKeyBase58 depend on which depend on
 * what type of key it is.
 *
 * @param
 *      builder               [in] A handle to DIDDocument Builder.
 * @param
 *      keyid                 [in] An identifier of public key.
 * @param
 *      controller            [in] A controller property, identifies
 *                              the controller of the corresponding private key.
 * @param
 *      key                  [in] Key propertie depend on key type.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocumentBuilder_AddPublicKey(DIDDocumentBuilder *builder,
        DIDURL *keyid, DID *controller, const char *key);

/**
 * \~English
 * Remove specified public key from DID Document.
 *
 * @param
 *      builder               [in] A handle to DIDDocument Builder.
 * @param
 *      keyid                 [in] An identifier of public key.
 * @param
 *      force                 [in] True, must to remove key; false, if key
 *                                 is authentication or authorization key, not to remove.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocumentBuilder_RemovePublicKey(DIDDocumentBuilder *builder,
        DIDURL *keyid, bool force);

/**
 * \~English
 * Add public key to Authenticate.
 * Authentication is the mechanism by which the controller(s) of a DID can
 * cryptographically prove that they are associated with that DID.
 * A DID Document must include an authentication property.
 *
 * @param
 *      builder               [in] A handle to DIDDocument Builder.
 * @param
 *      keyid                 [in] An identifier of public key.
 * @param
 *      key                   [in] Key property depend on key type.
 *                             If 'keyid' is from pk array, 'key' can be null.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocumentBuilder_AddAuthenticationKey(DIDDocumentBuilder *builder,
        DIDURL *keyid, const char *key);

/**
 * \~English
 * Remove authentication key from Authenticate.
 *
 * @param
 *      builder              [in] A handle to DIDDocument Builder.
 * @param
 *      keyid                [in] An identifier of public key.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocumentBuilder_RemoveAuthenticationKey(DIDDocumentBuilder *builder,
        DIDURL *keyid);

/**
 * \~English
 * Add public key to authorizate.
 * Authorization is the mechanism used to state
 * how operations may be performed on behalf of the DID subject.
 *
 * @param
 *      builder              [in] A handle to DIDDocument Builder.
 * @param
 *      keyid                [in] An identifier of authorization key.
 * @param
 *      controller           [in] A controller property, identifies
 *                              the controller of the corresponding private key.
 *                              If 'keyid' is from pk array, 'controller' can be null.
 * @param
 *      key                  [in] Key property depend on key type.
 *                              If 'keyid' is from pk array, 'key' can be null.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocumentBuilder_AddAuthorizationKey(DIDDocumentBuilder *builder,
        DIDURL *keyid, DID *controller, const char *key);

/**
 * \~English
 * Add Authorization key to Authentication array according to DID.
 * Authentication is the mechanism by which the controller(s) of a DID can
 * cryptographically prove that they are associated with that DID.
 * A DID Document must include an authentication property.
 *
 * @param
 *      builder              [in] A handle to DIDDocument Builder.
 * @param
 *      keyid                [in] An identifier of public key.
 * @param
 *      controller           [in] A controller property, identifies
 *                              the controller of the corresponding private key.
 * @param
 *      authorkeyid          [in] An identifier of authorization key.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocumentBuilder_AuthorizationDid(DIDDocumentBuilder *builder,
        DIDURL *keyid, DID *controller, DIDURL *authorkeyid);

/**
 * \~English
 * Remove authorization key from authorizate.
 *
 * @param
 *      builder               [in] A handle to DIDDocument Builder.
 * @param
 *      keyid                 [in] An identifier of authorization key.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocumentBuilder_RemoveAuthorizationKey(DIDDocumentBuilder *builder,
        DIDURL *keyid);

/**
 * \~English
 * Add one credential to credential array.
 *
 * @param
 *      builder              [in] A handle to DIDDocument Builder.
 * @param
 *      credential           [in] The handle to Credential.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocumentBuilder_AddCredential(DIDDocumentBuilder *builder,
        Credential *credential);
/**
 * \~English
 * Directly, add self claimed information(credential).
 *
 * @param
 *      builder              [in] A handle to DIDDocument Builder.
 * @param
 *      credid               [in] The handle to DIDURL.
 * @param
 *      types                [in] The array of credential types.
 *                                Support types == NULL，api add 'SelfProclaimedCredential' type.
 * @param
 *      typesize             [in] The size of credential types.
 * @param
 *      properties           [in] The array of credential subject property.
 * @param
 *      propsize             [in] The size of credential subject property.
 * @param
 *      expires              [in] The time to credential be expired.
 *                               Support expires == 0, api add document expires time.
 * @param
 *      storepass            [in] Password for DIDStores.
 * @return
 *      If no error occurs, return 0.
 *      Otherwise, return -1.
 */
DID_API int DIDDocumentBuilder_AddSelfClaimedCredential(DIDDocumentBuilder *builder,
        DIDURL *credid, const char **types, size_t typesize,
        Property *properties, int propsize, time_t expires, const char *storepass);
/**
 * \~English
 * Remove specified credential from credential array.
 *
 * @param
 *      builder              [in] A handle to DIDDocument Builder.
 * @param
 *      credid               [in] An identifier of Credential.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocumentBuilder_RemoveCredential(DIDDocumentBuilder *builder,
        DIDURL *credid);

/**
 * \~English
 * Add one Service to services array.
 *
 * @param
 *      builder              [in] A handle to DIDDocument Builder.
 * @param
 *      serviceid            [in] The identifier of Service.
 * @param
 *      type                 [in] The type of Service.
 * @param
 *      endpoint             [in] ServiceEndpoint property is a valid URI.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocumentBuilder_AddService(DIDDocumentBuilder *builder,
        DIDURL *serviceid, const char *type, const char *endpoint);

/**
 * \~English
 * Remove specified Service to services array.
 *
 * @param
 *      builder              [in] A handle to DIDDocument Builder.
 * @param
 *      serviceid            [in] The identifier of Service.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocumentBuilder_RemoveService(DIDDocumentBuilder *builder,
        DIDURL *serviceid);

/**
 * \~English
 * Set expire time about DID Document.
 *
 * @param
 *      builder             [in] A handle to DIDDocument Builder.
 * @param
 *      expires             [in] time to expire.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocumentBuilder_SetExpires(DIDDocumentBuilder *builder, time_t expires);

/**
 * \~English
 * Get the count of public keys. A DID Document must include a publicKey property.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @return
 *      size of public keys on success, -1 if an error occurred.
 */
DID_API ssize_t DIDDocument_GetPublicKeyCount(DIDDocument *document);

/**
 * \~English
 * Get the array of public keys. A DID Document MAY include a publicKey property.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      pks                  [out] The buffer that will receive the public keys.
 * @param
 *      size                 [in] The buffer size of pks.
 * @return
 *      size of public keys on success, -1 if an error occurred.
 */
DID_API ssize_t DIDDocument_GetPublicKeys(DIDDocument *document,
        PublicKey **pks, size_t size);

/**
 * \~English
 * Get public key according to identifier of public key.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      keyid                [in] An identifier of public key.
 * @return
 *      If no error occurs, return the handle to public key.
 *      Otherwise, return NULL
 */
DID_API PublicKey *DIDDocument_GetPublicKey(DIDDocument *document, DIDURL *keyid);

/**
 * \~English
 * Get public key conforming to type or identifier.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      type                 [in] The type of public key to be selected.
 * @param
 *      keyid                [in] An identifier of public key to be selected.
 * @param
 *      pks                  [out] The buffer that will receive the public keys.
 * @param
 *      size                 [in] The buffer size of pks.
 * @return
 *      size of public keys selected on success, -1 if an error occurred.
 */
DID_API ssize_t DIDDocument_SelectPublicKeys(DIDDocument *document, const char *type,
        DIDURL *keyid, PublicKey **pks, size_t size);

/**
 * \~English
 * Get primary public key, which is for creating method specific string.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @return
 *      If no error occurs, return the handle to identifier.
 *      Otherwise, return NULL
 */
DID_API DIDURL *DIDDocument_GetDefaultPublicKey(DIDDocument *document);

/**
 * \~English
 * Get the count of authentication keys.
 * A DID Document must include a authentication property.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @return
 *      size of authentication keys on success, -1 if an error occurred.
 */
DID_API ssize_t DIDDocument_GetAuthenticationCount(DIDDocument *document);

/**
 * \~English
 * Get the array of authentication keys.
 * A DID Document must include a authentication property.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      pks                  [out] The buffer that will receive
 *                                 the authentication keys.
 * @param
 *      size                 [in] The buffer size of pks.
 * @return
 *      size of authentication keys on success, -1 if an error occurred.
 */
DID_API ssize_t DIDDocument_GetAuthenticationKeys(DIDDocument *document,
        PublicKey **pks, size_t size);

/**
 * \~English
 * Get authentication key according to identifier of authentication key.
 * A DID Document must include a authentication property.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      keyid                [in] An identifier of authentication key.
 * @return
 *       If no error occurs, return the handle to public key.
 *       Otherwise, return NULL
 */
DID_API PublicKey *DIDDocument_GetAuthenticationKey(DIDDocument *document, DIDURL *keyid);

/**
 * \~English
 * Get authentication key conforming to type or identifier of key.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      type                 [in] The type of authentication key to be selected.
 * @param
 *      keyid                [in] An identifier of authentication key to be selected.
 * @param
 *      pks                  [out] The buffer that will receive the authentication keys.
 * @param
 *      size                 [in] The buffer size of pks.
 * @return
 *      size of authentication key selected, -1 if an error occurred.
 */
DID_API ssize_t DIDDocument_SelectAuthenticationKeys(DIDDocument *document, const char *type,
        DIDURL *keyid, PublicKey **pks, size_t size);


/**
 * \~English
 * Check key if authentiacation key or not.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      keyid                [in] An identifier of authentication key.
 * @return
 *      true if has authentication key, or false.
 */
DID_API bool DIDDocument_IsAuthenticationKey(DIDDocument *document, DIDURL *keyid);

/**
 * \~English
 * Check key if authorization key or not.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      keyid                [in] An identifier of authorization key.
 * @return
 *      true if has authorization key, or false.
 */
DID_API bool DIDDocument_IsAuthorizationKey(DIDDocument *document, DIDURL *keyid);

/**
 * \~English
 * Get the count of authorization keys.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @return
 *      size of authorization keys on success, -1 if an error occurred.
 */
DID_API ssize_t DIDDocument_GetAuthorizationCount(DIDDocument *document);

/**
 * \~English
 * Get the array of authorization keys.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      pks                  [out] The buffer that will receive
 *                                 the authorization keys.
 * @param
 *      size                 [in] The buffer size of pks.
 * @return
 *      size of authorization keys on success, -1 if an error occurred.
 */
DID_API ssize_t DIDDocument_GetAuthorizationKeys(DIDDocument *document,
        PublicKey **pks, size_t size);

/**
 * \~English
 * Get authorization key according to identifier of key.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      keyid                [in] An identifier of authorization key.
 * @return
 *       If no error occurs, return the handle to public key.
 *       Otherwise, return NULL
 */
DID_API PublicKey *DIDDocument_GetAuthorizationKey(DIDDocument *document, DIDURL *keyid);

/**
 * \~English
 * Get authorization key conforming to type or identifier of key.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      type                 [in] The type of authorization key to be selected.
 * @param
 *      keyid                [in] An identifier of authorization key to be selected.
 * @param
 *      pks                  [out] The buffer that will receive the authorization keys.
 * @param
 *      size                 [in] The buffer size of pks.
 * @return
 *      size of authorization key selected, -1 if an error occurred.
 */
DID_API ssize_t DIDDocument_SelectAuthorizationKeys(DIDDocument *document, const char *type,
        DIDURL *keyid, PublicKey **pks, size_t size);


/**
 * \~English
 * Get the count of credentials.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @return
 *      size of credentials on success, -1 if an error occurred.
 */
DID_API ssize_t DIDDocument_GetCredentialCount(DIDDocument *document);

/**
 * \~English
 * Get the array of credentials.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      creds                [out] The buffer that will receive credentials.
 * @param
 *      size                 [in] The buffer size of creds.
 * @return
 *      size of credentials on success, -1 if an error occurred.
 */
DID_API ssize_t DIDDocument_GetCredentials(DIDDocument *document,
        Credential **creds, size_t size);

/**
 * \~English
 * Get credential according to identifier of credential.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      credid               [in] An identifier of Credential.
 * @return
 *       If no error occurs, return the handle to Credential.
 *       Otherwise, return NULL
 */
DID_API Credential *DIDDocument_GetCredential(DIDDocument *document, DIDURL *credid);

/**
 * \~English
 * Get Credential conforming to type or identifier of key.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      type                 [in] The type of Credential.
 * @param
 *      credid               [in] An identifier of Credential to be selected.
 * @param
 *      creds                [out] The buffer that will receive credentials.
 * @param
 *      size                 [in] The buffer size of creds.
 * @return
 *      size of credentials selected, -1 if an error occurred.
 */
DID_API ssize_t DIDDocument_SelectCredentials(DIDDocument *document, const char *type,
        DIDURL *credid, Credential **creds, size_t size);


/**
 * \~English
 * Get the count of services.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @return
 *      size of services on success, -1 if an error occurred.
 */
DID_API ssize_t DIDDocument_GetServiceCount(DIDDocument *document);

/**
 * \~English
 * Get the array of services.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      services             [out] The buffer that will receive services.
 * @param
 *      size                 [in] The buffer size of services.
 * @return
 *      size of services on success, -1 if an error occurred.
 */
DID_API ssize_t DIDDocument_GetServices(DIDDocument *document, Service **services,
        size_t size);

/**
 * \~English
 * Get Service according to identifier of Service.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      serviceid            [in] An identifier of Service.
 * @return
 *       If no error occurs, return the handle to Service.
 *       Otherwise, return NULL
 */
DID_API Service *DIDDocument_GetService(DIDDocument *document, DIDURL *serviceid);

/**
 * \~English
 * Get Service conforming to type or identifier of key.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      type                 [in] The type of Service.
 * @param
 *      serviceid            [in] An identifier of Service to be selected.
 * @param
 *      services             [out] The buffer that will receive services.
 * @param
 *      size                 [in] The buffer size of services.
 * @return
 *      size of services selected, -1 if an error occurred.
 */
DID_API ssize_t DIDDocument_SelectServices(DIDDocument *document, const char *type,
        DIDURL *serviceid, Service **services, size_t size);

/**
 * \~English
 * Get expire time about DID Document.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @return
 *      expire time on success, 0 if failed.
 */
DID_API time_t DIDDocument_GetExpires(DIDDocument *document);

/**
 * \~English
 * Sign data by DID.
 *
 * @param
 *      document                 [in] The handle to DID Document.
 * @param
 *      keyid                    [in] Public key to sign.
 *                                   If key = NULL, sdk will get default key from
 *                                   DID Document.
 * @param
 *      storepass                [in] Pass word to sign.
 * @param
 *      sig                      [out] The buffer will receive signature data.
 * @param
 *      count                    [in] The size of data list.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocument_Sign(DIDDocument *document, DIDURL *keyid, const char *storepass,
        char *sig, int count, ...);

/**
 * \~English
 * Sign digest by DID.
 *
 * @param
 *      document                 [in] The handle to DID Document.
 * @param
 *      keyid                    [in] Public key to sign.
 *                               If keyid is null, then will sign with
 *                               the default key of this DID document.
 * @param
 *      storepass                [in] Pass word to sign.
 * @param
 *      sig                      [out] The buffer will receive signature data.
 * @param
 *      digest                   [in] The digest to sign.
  * @param
 *      size                     [in] The length of digest array.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocument_SignDigest(DIDDocument *document, DIDURL *keyid,
        const char *storepass, char *sig, uint8_t *digest, size_t size);

/**
 * \~English
 * verify data.
 *
 * @param
 *      document                [in] The handle to DID Document.
 * @param
 *      keyid                   [in] Public key to sign.
 *                                   If key = NULL, sdk will get default key from
 *                                   DID Document.
 * @param
 *      sig                     [in] Signature data.
 * @param
 *      count                   [in] The size of data list.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocument_Verify(DIDDocument *document, DIDURL *keyid, char *sig,
        int count, ...);
/**
 * \~English
 * verify digest.
 *
 * @param
 *      document                [in] The handle to DID Document.
 * @param
 *      keyid                   [in] Public key to sign.
 *                                   If key = NULL, sdk will get default key from
 *                                   DID Document.
 * @param
 *      sig                     [in] Signature data.
 * @param
 *      digest                   [in] The digest to sign.
  * @param
 *      size                     [in] The length of digest array.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocument_VerifyDigest(DIDDocument *document, DIDURL *keyid,
        char *sig, uint8_t *digest, size_t size);

/**
 * \~English
 * Get DIDMetaData from DID.
 *
 * @param
 *      document                 [in] The handle to DIDDocument.
 * @return
 *      If no error occurs, return the handle to DIDMetadata.
 *      Otherwise, return NULL.
 */
DID_API DIDMetaData *DIDDocument_GetMetaData(DIDDocument *document);

/**
 * \~English
 * Save DIDMetaData of document.
 *
 * @param
 *      document                    [in] The handle to DIDDocument.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocument_SaveMetaData(DIDDocument *document);

/**
 * \~English
 * Get the type property of embedded proof.
 *
 * @param
 *      document                 [in] A handle to DID Document.
 * @return
 *      If no error occurs, return type string.
 *      Otherwise, return NULL.
 */
DID_API const char *DIDDocument_GetProofType(DIDDocument *document);

/**
 * \~English
 * Get verification method identifier of DIDDocument.
 * The verification Method property specifies the public key
 * that can be used to verify the digital signature.
 *
 * @param
 *      document                 [in] A handle to DID Document.
 * @return
 *      If no error occurs, return the handle to identifier of public key.
 *      Otherwise, return NULL.
 */
DID_API DIDURL *DIDDocument_GetProofCreater(DIDDocument *document);

/**
 * \~English
 * Get time of create DIDDocument proof.
 *
 * @param
 *      document                 [in] A handle to DID Document.
 * @return
 *      If no error occurs, return 0.
 *      Otherwise, return time.
 */
DID_API time_t DIDDocument_GetProofCreatedTime(DIDDocument *document);

/**
 * \~English
 * Get signature of DIDDocument.
 * A signature that can be later used to verify the authenticity and
 * integrity of a linked data document.
 *
 * @param
 *      document                 [in] A handle to DID Document.
 * @return
 *      If no error occurs, return signature string.
 *      Otherwise, return NULL.
 */
DID_API const char *DIDDocument_GetProofSignature(DIDDocument *document);

/**
 * \~English
 * Get JWTBuilder from document.
 *
 * @param
 *      document                 [in] A handle to DID Document.
 *                                ps：document must attatch DIDstore.
 * @return
 *      If no error occurs, return the handle to JWTBuilder.
 *      Otherwise, return NULL.
 *      Notice that user need to release the handle of returned instance to destroy it's memory.
 */
DID_API JWTBuilder *DIDDocument_GetJwtBuilder(DIDDocument *document);
/**
 * \~English
 * Get identifier of public key.
 *
 * @param
 *      publickey             [in] A handle to public key.
 * @return
 *      If no error occurs, return the identifier of public key.
 *      Otherwise, return NULL.
 */

DID_API DIDURL *PublicKey_GetId(PublicKey *publickey);

/**
 * \~English
 * Get DID controller of public key.
 *
 * @param
 *      publickey             [in] A handle to public key.
 * @return
 *      If no error occurs, return the handle to DID controller.
 *      Otherwise, return NULL.
 */
DID_API DID *PublicKey_GetController(PublicKey *publickey);

/**
 * \~English
 * Get key property of public key.
 *
 * @param
 *      publickey             [in] A handle to public key.
 * @return
 *      If no error occurs, return key property string.
 *      Otherwise, return NULL.
 */
DID_API const char *PublicKey_GetPublicKeyBase58(PublicKey *publickey);

/**
 * \~English
 * Get type of public key.
 *
 * @param
 *      publickey             [in] A handle to public key.
 * @return
 *      If no error occurs, return key type string.
 *      Otherwise, return NULL.
 */
DID_API const char *PublicKey_GetType(PublicKey *publickey);

/**
 * \~English
 * Publickey is authentication key or not.
 *
 * @param
 *      publickey             [in] A handle to public key.
 * @return
 *      If publickey is authentication key, return true.
 *      Otherwise, return false.
 */
DID_API bool PublicKey_IsAuthenticationKey(PublicKey *publickey);

/**
 * \~English
 * Publickey is authorization key or not.
 *
 * @param
 *      publickey             [in] A handle to public key.
 * @return
 *      If publickey is authorization key, return true.
 *      Otherwise, return false.
 */
DID_API bool PublicKey_IsAuthorizationKey(PublicKey *publickey);

/**
 * \~English
 * Get identifier of Service.
 *
 * @param
 *      service             [in] A handle to Service.
 * @return
 *      If no error occurs, return identifier of service.
 *      Otherwise, return NULL.
 */
DID_API DIDURL *Service_GetId(Service *service);

/**
 * \~English
 * Get service end point.
 *
 * @param
 *      service             [in] A handle to Service.
 * @return
 *      If no error occurs, return service point string.
 *      Otherwise, return NULL.
 */
DID_API const char *Service_GetEndpoint(Service *service);

/**
 * \~English
 * Get type of service.
 *
 * @param
 *      service             [in] A handle to Service.
 * @return
 *      If no error occurs, return service type string.
 *      Otherwise, return NULL.
 */
DID_API const char *Service_GetType(Service *service);

/******************************************************************************
 * Credential
 *****************************************************************************/
/**
 * \~English
 * Get json non-formatted context from Credential.
 *
 * @param
 *      cred                 [in] A handle to Credential.
 * @param
 *      normalized           [in] Json context is normalized or not.
 *                           true represents normalized, false represents not.
 * @return
 *      If no error occurs, return json context. Otherwise, return NULL.
 *      Notice that user need to free the returned value that it's memory.
 */
DID_API const char *Credential_ToJson(Credential *cred, bool normalized);

/**
 * \~English
 * Get json formatted context from Credential.
 *
 * @param
 *      cred                 [in] A handle to Credential.
 * @param
 *      normalized           [in] Json context is normalized or not.
 *                           true represents normalized, false represents not.
 * @return
 *      If no error occurs, return json context. Otherwise, return NULL.
 *      Notice that user need to free the returned value that it's memory.
 */
DID_API const char *Credential_ToString(Credential *cred, bool normalized);

/**
 * \~English
 * Get one DID's Credential from json context.
 *
 * @param
 *      json                 [in] Json context about credential.
 * @param
 *      owner                  [in] A handle to credential owner's DID.
 * @return
 *      If no error occurs, return the handle to Credential.
 *      Otherwise, return NULL.
 *      Notice that user need to release the handle of returned instance to destroy it's memory.
 */
DID_API Credential *Credential_FromJson(const char *json, DID *owner);

/**
 * \~English
 * Destroy Credential.
 *
 * @param
 *      cred                 [in] A handle to Credential.
 */
DID_API void Credential_Destroy(Credential *cred);

/**
 * \~English
 * Check Credential is self claimed or not.
 *
 * @param
 *      cred                 [in] A handle to Credential.
 * @return
 *      true if Credential is self claimed.
 *      Otherwise, return false.
 */
DID_API bool Credential_IsSelfProclaimed(Credential *cred);

/**
 * \~English
 * Get id property from Credential.
 *
 * @param
 *      cred                 [in] A handle to Credential.
 * @return
 *      If no error occurs, return id property of credential.
 *      Otherwise, return NULL.
 */
DID_API DIDURL *Credential_GetId(Credential *cred);

/**
 * \~English
 * Get who this credential is belong to.
 *
 * @param
 *      cred                 [in] A handle to Credential.
 * @return
 *      If no error occurs, return owner DID.
 *      Otherwise, return NULL.
 */
DID_API DID *Credential_GetOwner(Credential *cred);

/**
 * \~English
 * Get count of Credential types.
 *
 * @param
 *      cred                 [in] A handle to Credential.
 * @return
 *      size of Credential types on success, -1 if an error occurred.
 */
DID_API ssize_t Credential_GetTypeCount(Credential *cred);

/**
 * \~English
 * Get array of Credential types.
 *
 * @param
 *      cred                 [in] A handle to Credential.
 * @param
 *      types                [out] The buffer that will receive credential types.
  * @param
 *      size                 [in] The buffer size of credential types.
 * @return
 *      size of Credential types on success, -1 if an error occurred.
 */
DID_API ssize_t Credential_GetTypes(Credential *cred, const char **types, size_t size);

/**
 * \~English
 * Get DID issuer of Credential.
 *
 * @param
 *      cred                 [in] A handle to Credential.
 * @return
 *      If no error occurs, return the handle to DID issuer.
 *      Otherwise, return NULL.
 */
DID_API DID *Credential_GetIssuer(Credential *cred);

/**
 * \~English
 * Get date of issuing credential.
 *
 * @param
 *      cred                 [in] A handle to Credential.
 * @return
 *      If no error occurs, return the date.
 *      Otherwise, return 0.
 */
DID_API time_t Credential_GetIssuanceDate(Credential *cred);

/**
 * \~English
 * Get the date of credential expired.
 *
 * @param
 *      cred                 [in] A handle to Credential.
 * @return
 *      If no error occurs, return the time.
 *      Otherwise, return 0.
 */
DID_API time_t Credential_GetExpirationDate(Credential *cred);

/**
 * \~English
 * Get size of subject properties in Credential.
 * A credential must have a credential Subject property. The value of
 * the credential Subject property is defined as a set of objects that
 * contain one or more properties that are each related to a subject
 * of the credential. Each object must contain an id.
 *
 * @param
 *      cred                 [in] A handle to Credential.
 * @return
 *      size of subject porperties on success, -1 if an error occurred.
 */
DID_API ssize_t Credential_GetPropertyCount(Credential *cred);

/**
 * \~English
 * Get array of subject properties in Credential.
 *
 * @param
 *      cred                 [in] A handle to Credential.
 * @return
 *      size of subject porperties on success, -1 if an error occurred.
 *      Notice that user need to free the returned value it's memory.
 */
DID_API const char *Credential_GetProperties(Credential *cred);

/**
 * \~English
 * Get specific subject property value in string with the given key of property.
 *
 * @param
 *      cred                 [in] A handle to Credential.
 * @param
 *      name                 [in] The key of property.
 * @return
 *      If no error occurs, return property value string, otherwise return NULL.
 *      Notice that user need to free the returned value it's memory.
 */
DID_API const char *Credential_GetProperty(Credential *cred, const char *name);

/**
 * \~English
 * Get verification method identifier of Credential.
 * The verification Method property specifies the public key
 * that can be used to verify the digital signature.
 *
 * @param
 *      cred                 [in] A handle to Credential.
 * @return
 *      If no error occurs, return the handle to identifier of public key.
 *      Otherwise, return NULL.
 */
DID_API DIDURL *Credential_GetProofMethod(Credential *cred);

/**
 * \~English
 * Get the type property of embedded proof.
 *
 * @param
 *      cred                 [in] A handle to Credential.
 * @return
 *      If no error occurs, return type string.
 *      Otherwise, return NULL.
 */
DID_API const char *Credential_GetProofType(Credential *cred);

/**
 * \~English
 * Get signature of Credential.
 * A signature that can be later used to verify the authenticity and
 * integrity of a linked data document.
 *
 * @param
 *      cred                 [in] A handle to Credential.
 * @return
 *      If no error occurs, return signature string.
 *      Otherwise, return NULL.
 */
DID_API const char *Credential_GetProofSignture(Credential *cred);

/**
 * \~English
 * Credential is expired or not.
 * Issuance always occurs before any other actions involving a credential.
 *
 * @param
 *      cred                      [in] The Credential handle.
 * @return
 *      flase if not expired, true if expired.
 */
DID_API bool Credential_IsExpired(Credential *cred);

/**
 * \~English
 * Credential is genuine or not.
 * Issuance always occurs before any other actions involving a credential.
 *
 * @param
 *      cred                      [in] The Credential handle.
 * @return
 *      flase if not genuine, true if genuine.
 */
DID_API bool Credential_IsGenuine(Credential *cred);

/**
 * \~English
 * Credential is expired or not.
 * Issuance always occurs before any other actions involving a credential.
 *
 * @param
 *      cred                      [in] The Credential handle.
 * @return
 *      flase if not valid, true if valid.
 */
DID_API bool Credential_IsValid(Credential *cred);

/**
 * \~English
 * Set Credential from DID Store.
 *
 * @param
 *      credential               [in] The handle to Credential.
 * @param
 *      alias                    [in] The nickname of credential.
 * @return
*      0 on success, -1 if an error occurred.
 */
DID_API int Credential_SaveMetaData(Credential *credential);

/**
 * \~English
 * Get credential alias.
 *
 * @param
 *      credential                [in] The handle to Credential.
 * @return
 *      If no error occurs, return alias string.
 *      Otherwise, return NULL.
 */
DID_API CredentialMetaData *Credential_GetMetaData(Credential *credential);

/******************************************************************************
 * Issuer
 *****************************************************************************/
/**
 * \~English
 * Create a issuer to issue Credential.
 *
 * @param
 *      did                      [in] Issuer's did.
 * @param
 *      signkey                  [in] Issuer's key to sign credential.
 * @param
 *      store                    [in] The handle to DIDStore.
 * @return
 *      If no error occurs, return the handle to Issuer. Otherwise, return NULL.
 *      Notice that user need to release the handle of returned instance to destroy it's memory.
 */
DID_API Issuer *Issuer_Create(DID *did, DIDURL *signkey, DIDStore *store);

/**
 * \~English
 * Destroy a issuer.
 *
 * @param
 *      issuer                    [in] the handle of Issuer..
 */
DID_API void Issuer_Destroy(Issuer *issuer);

/**
 * \~English
 * An issuer issues a verifiable credential to a holder with subject object.
 *
 * @param
 *      issuer               [in] An issuer issues this credential.
 * @param
 *      owner                [in] A handle to DID.
 *                               The holder of this Credential.
 * @param
 *      credid               [in] The handle to DIDURL.
 * @param
 *      types                [in] The array of credential types.
 * @param
 *      typesize             [in] The size of credential types.
 * @param
 *      subject              [in] The array of credential subject property.
 * @param
 *      size                 [in] The size of credential subject property.
 * @param
 *      expires              [in] The time to credential be expired.
 * @param
 *      storepass            [in] The password for DIDStore.
 * @return
 *      If no error occurs, return the handle to Credential issued.
 *      Otherwise, return NULL.
 *      Notice that user need to release the handle of returned instance to destroy it's memory.
 */
DID_API Credential *Issuer_CreateCredential(Issuer *issuer, DID *owner, DIDURL *credid,
        const char **types, size_t typesize, Property *subject, int size,
        time_t expires, const char *storepass);

/**
 * \~English
 * An issuer issues a verifiable credential to a holder with subject string.
 *
 * @param
 *      issuer               [in] An issuer issues this credential.
 * @param
 *      owner                [in] A handle to DID.
 *                               The holder of this Credential.
 * @param
 *      credid               [in] The handle to DIDURL.
 * @param
 *      types                [in] The array of credential types.
 * @param
 *      typesize             [in] The size of credential types.
 * @param
 *      subject              [in] The array of credential subject property.
 * @param
 *      expires              [in] The time to credential be expired.
 * @param
 *      storepass            [in] The password for DIDStore.
 * @return
 *      If no error occurs, return the handle to Credential issued.
 *      Otherwise, return NULL.
 *      Notice that user need to release the handle of returned instance to destroy it's memory.
 */
DID_API Credential *Issuer_CreateCredentialByString(Issuer *issuer, DID *owner,
        DIDURL *credid, const char **types, size_t typesize, const char *subject,
        time_t expires, const char *storepass);

/**
 * \~English
 * Get the DID of this issuer
 *
 * @param
 *      issuer                  [in] The handle to Issuer.
 * @return
 *      If no error occurs, return the handle to DID of this issuer.
 *      Otherwise, return NULL.
 */
DID_API DID *Issuer_GetSigner(Issuer *issuer);

/**
 * \~English
 * Get the DID of this issuer
 *
 * @param
 *      issuer                  [in] The handle to Issuer.
 * @return
 *      If no error occurs, return the handle to key.
 *      Otherwise, return NULL.
 */
DID_API DIDURL *Issuer_GetSignKey(Issuer *issuer);

/******************************************************************************
 * DIDStore
 *****************************************************************************/
/**
 * \~English
 * Initialize or check the DIDStore.
 *
 * @param
 *      root                 [in] The path of DIDStore's root.
 * @param
 *      adapter              [in] The handle to DIDAdapter.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API DIDStore* DIDStore_Open(const char *root, DIDAdapter *adapter);

/**
 * \~English
 * Deinitialize DIDStore. * @param
 *      store                 [in] The handle to DIDStore.
 * @param
 *      store                 [in] The handle to DIDStore.
 */
DID_API void DIDStore_Close(DIDStore *store);

/**
 * \~English
 * Check if it has private identity or not.
 *
 * @param
 *      store                 [in] The handle to DIDStore.
 * @return
 *      ture if it has identity, false if it has not.
 */
DID_API bool DIDStore_ContainsPrivateIdentity(DIDStore *store);

/**
 * \~English
 * Initial user's private identity by mnemonic.
 *
  * @param
 *      store             [in] THe handle to DIDStore.
 * @param
 *      storepass         [in] The password for DIDStore.
 * @param
 *      mnemonic          [in] Mnemonic for generate key.
 * @param
 *      passphrase        [in] The pass word to generate private identity.
 * @param
 *      language          [in] The language for DID.
 *                        support language string: "chinese_simplified",
 *                        "chinese_traditional", "czech", "english", "french",
 *                        "italian", "japanese", "korean", "spanish".
 * @param
 *      force             [in] If private identity exist, remove or remain it.
 *                        If force is true, then will choose to create a new identity
 *                        even if the private identity already exists and
 *                        the new private key will replace the original one in DIDStore.
 *                        If force is false, then will choose to remain the old
 *                        private key if the private identity exists, and return error code.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_InitPrivateIdentity(DIDStore *store, const char *storepass,
        const char *mnemonic, const char *passphrase, const char *language, bool force);

/**
 * \~English
 * Initial user's identity by e.
 *
  * @param
 *      store             [in] The handle to DIDStore.
 * @param
 *      storepass         [in] The password for DIDStore.
 * @param
 *      extendedkey       [in] Extendedkey string.
 * @param
 *      force             [in] If private identity exist, remove or remain it.
 *                        If force is true, then will choose to create a new identity
 *                        even if the private identity already exists and
 *                        the new private key will replace the original one in DIDStore.
 *                        If force is false, then will choose to remain the old
 *                        private key if the private identity exists, and return error code.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_InitPrivateIdentityFromRootKey(DIDStore *store,
        const char *storepass, const char *extendedkey, bool force);

/**
 * \~English
 * Synchronize DIDStore.
 *
  * @param
 *      store                  [in] THe handle to DIDStore.
 * @param
 *      storepass              [in] The pass word of DID holder.
 * @param
 *      callback               [in] The method to merge document.
 *                              callback == NULL, use default method supported by sdk.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_Synchronize(DIDStore *store, const char *storepass,
        DIDStore_MergeCallback *callback);

/**
 * \~English
 * Create new DID Document and store in the DID Store.
 *
 * @param
 *      store                     [in] THe handle to DIDStore.
 * @param
 *      storepass                 [in] Password for DIDStore.
 * @param
 *      alias                     [in] The nickname of DID.
 *                                     ‘alias' supports NULL.
 * @return
 *      If no error occurs, return the handle to DID Document.
 *      Otherwise, return NULL.
 *      Notice that user need to release the handle of returned instance to destroy it's memory.
 */
DID_API DIDDocument *DIDStore_NewDID(DIDStore *store, const char *storepass,
        const char *alias);

/**
 * \~English
 * Create new DID document and store it in the DID store with given index.
 *
 * @param
 *      store                     [in] THe handle to DIDStore.
 * @param
 *      storepass                 [in] Password for DIDStore.
 * @param
 *      index                     [in] The index to create new did.
 * @param
 *      alias                     [in] The nickname of DID.
 *                                     'alias' supports NULL.
 * @return
 *      If no error occurs, return the handle to DID Document.
 *      Otherwise, return NULL.
 *      Notice that user need to release the handle of returned instance to destroy it's memory.
 */
DID_API DIDDocument *DIDStore_NewDIDByIndex(DIDStore *store, const char *storepass,
        int index, const char *alias);

/**
 * \~English
 * Only get DID object by index, not create document and so on.
 *
 * @param
 *      store                     [in] THe handle to DIDStore.
 * @param
 *      index                     [int] The index of DerivedKey from HDKey.
 * @return
 *      If no error occurs, return DID object. Free DID after use it.
 *      Otherwise, return NULL.
 */
DID_API DID *DIDStore_GetDIDByIndex(DIDStore *store, int index);
/**
 * \~English
 * Create new DID Document and store in the DID Store.
 *
 * @param
 *      store              [in] THe handle to DIDStore.
 * @param
 *      storepass          [in] The password of DIDStore.
 * @param
 *      mnemonic           [out] The buffer that will receive the mnemonic.
 *                               The buffer size should at least
 *                               (ELA_MAX_ADDRESS_LEN + 1) bytes.
 * @param
 *      size               [in] The buffter size.
 * @return
 *      If no error occurs, return the handle to DID Document.
 *      Otherwise, return NULL.
 */
DID_API int DIDStore_ExportMnemonic(DIDStore *store, const char *storepass,
        char *mnemonic, size_t size);

/**
 * \~English
 * Store DID Document in DID Store.
 *
 * @param
 *      store                     [in] The handle to DIDStore.
 * @param
 *      document                  [in] The handle to DID Document.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_StoreDID(DIDStore *store, DIDDocument *document);

/**
 * \~English
 * Load DID Document from DID Store.
 *
 * @param
 *      store                   [in] The handle to DIDStore.
 * @param
 *      did                     [in] The handle to DID.
 * @return
 *      If no error occurs, return the handle to DID Document.
 *      Otherwise, return NULL.
 *      Notice that user need to release the handle of returned instance to destroy it's memory.
 */
DID_API DIDDocument *DIDStore_LoadDID(DIDStore *store, DID *did);

/**
 * \~English
 * Check if contain specific DID or not.
 *
 * @param
 *      store                   [in] The handle to DIDStore.
 * @param
 *      did                     [in] The handle to DID.
 * @return
 *      true on success, false if an error occurred.
 */
DID_API bool DIDStore_ContainsDID(DIDStore *store, DID *did);

/**
 * \~English
 * Delete specific DID.
 *
 * @param
 *      store                   [in] The handle to DIDStore.
 * @param
 *      did                     [in] The handle to DID.
 * @return
 *      true on success, false if an error occurred.
 */
DID_API bool DIDStore_DeleteDID(DIDStore *store, DID *did);

/**
 * \~English
 * List DIDs in DID Store.
 *
 * @param
 *      store       [in] The handle to DIDStore.
 * @param
 *      filer       [in] DID filer. 0: all did; 1: did has privatekeys;
 *                                  2: did has no privatekeys.
 * @param
 *      callback    [in] a pointer to DIDStore_DIDsCallback function.
 * @param
 *      context     [in] the application defined context data.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_ListDIDs(DIDStore *store, ELA_DID_FILTER filer,
        DIDStore_DIDsCallback *callback, void *context);

/**
 * \~English
 * Store Credential in DID Store.
 *
 * @param
 *      store                    [in] The handle to DIDStore.
 * @param
 *      credential               [in] The handle to Credential.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_StoreCredential(DIDStore *store, Credential *credential);

/**
 * \~English
 * Load Credential from DID Store.
 *
 * @param
 *      store                   [in] The handle to DIDStore.
 * @param
 *      did                     [in] The handle to DID.
 * @param
 *      credid                   [in] The identifier of credential.
 * @return
 *      If no error occurs, return the handle to Credential.
 *      Otherwise, return NULL.
 *      Notice that user need to release the handle of returned instance to destroy it's memory.
 */
DID_API Credential *DIDStore_LoadCredential(DIDStore *store, DID *did, DIDURL *credid);

/**
 * \~English
 * Check if contain any credential of specific DID.
 *
 * @param
 *      store                   [in] The handle to DIDStore.
 * @param
 *      did                     [in] The handle to DID.
 * @return
 *      true on success, false if an error occurred.
 */
DID_API bool DIDStore_ContainsCredentials(DIDStore *store, DID *did);

/**
 * \~English
 * Check if contain specific credential of specific DID.
 *
 * @param
 *      store                   [in] The handle to DIDStore.
 * @param
 *      did                     [in] The handle to DID.
 * @param
 *      credid                  [in] The identifier of credential.
 * @return
 *      true on success, false if an error occurred.
 */
DID_API bool DIDStore_ContainsCredential(DIDStore *store, DID *did, DIDURL *credid);

/**
 * \~English
 * Delete specific credential.
 *
 * @param
 *      store                   [in] The handle to DIDStore.
 * @param
 *      did                     [in] The handle to DID.
 * @param
 *      id                      [in] The identifier of credential.
 * @return
 *      true on success, false if an error occurred.
 */
DID_API bool DIDStore_DeleteCredential(DIDStore *store, DID *did, DIDURL *id);

/**
 * \~English
 * List credentials of specific DID.
 *
 * @param
 *      store       [in] The handle to DIDStore.
 * @param
 *      did         [in] The handle to DID.
 * @param
 *      callback    [in] a pointer to DIDStore_CredentialsCallback function.
 * @param
 *      context     [in] the application defined context data.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_ListCredentials(DIDStore *store, DID *did,
        DIDStore_CredentialsCallback *callback, void *context);

/**
 * \~English
 * Get credential conforming to identifier or type property.
 *
 * @param
 *      store       [in] The handle to DIDStore.
 * @param
 *      did         [in] The handle to DID.
 * @param
 *      credid      [in] The identifier of credential.
 * @param
 *      type        [in] The type of Credential to be selected.
 * @param
 *      callback    [in] a pointer to DIDStore_CredentialsCallback function.
 * @param
 *      context     [in] the application defined context data.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_SelectCredentials(DIDStore *store, DID *did, DIDURL *credid,
        const char *type, DIDStore_CredentialsCallback *callback, void *context);

/**
 * \~English
 * Check if contain any private key of specific DID.
 *
 * @param
 *      store                   [in] The handle to DIDStore.
 * @param
 *      did                     [in] The handle to DID.
 * @return
 *      true on success, false if an error occurred.
 */
DID_API bool DIDSotre_ContainsPrivateKeys(DIDStore *store, DID *did);

/**
 * \~English
 * Check if contain specific private key of specific DID.
 *
 * @param
 *      store                   [in] The handle to DIDStore.
 * @param
 *      did                     [in] The handle to DID.
 * @param
 *      keyid                   [in] The identifier of public key.
 * @return
 *      true on success, false if an error occurred.
 */
DID_API bool DIDStore_ContainsPrivateKey(DIDStore *store, DID *did, DIDURL *keyid);

/**
 * \~English
 * Store private key.
 *
 * @param
 *      store                   [in] The handle to DIDStore.
 * @param
 *      storepass               [in] Password for DIDStore.
 * @param
 *      did                     [in] The handle to DID.
 * @param
 *      id                      [in] The handle to public key identifier.
 * @param
 *      privatekey              [in] Private key string.
 * @param
 *      size                    [in] The bytes of Private key.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_StorePrivateKey(DIDStore *store, const char *storepass,
        DID *did, DIDURL *id, const uint8_t *privatekey, size_t size);

/**
 * \~English
 * Delete private key.
 *
 * @param
 *      store                   [in] The handle to DIDStore.
 * @param
 *      did                     [in] The handle to DID.
 * @param
 *      keyid                    [in] The identifier of public key.
 */
DID_API void DIDStore_DeletePrivateKey(DIDStore *store, DID *did, DIDURL *keyid);

/**
 * \~English
 * Creates a DID and its associated DID Document to chain.
 *
 * @param
 *      store                    [in] The handle to DID Store.
 * @param
 *      storepass                [in] Pass word to sign.
 * @param
 *      did                      [in] The handle to DID.
 * @param
 *      signkey                  [in] The public key to sign.
 * @param
 *      force                    [in] Force document into chain.
 * @return
 *      true on success, false if an error occurred. Caller should free the returned value.
 */
DID_API bool DIDStore_PublishDID(DIDStore *store, const char *storepass,
        DID *did, DIDURL *signkey, bool force);

/**
 * \~English
 * Deactivate a DID on the chain.
 *
 * @param
 *      store                   [in] The handle to DIDStore.
 * @param
 *      storepass                [in] Password for DIDStore.
 * @param
 *      did                      [in] The handle to DID.
 * @param
 *      signkey                  [in] The public key to sign.
 * @return
 *      true on success, false if an error occurred. Caller should free the returned value.
 */
DID_API bool DIDStore_DeactivateDID(DIDStore *store, const char *storepass,
        DID *did, DIDURL *signkey);

/**
 * \~English
 * Change the store password from old one to new one.
 *
 * @param
 *      store                   [in] The handle to DIDStore.
 * @param
 *      new                     [in] New store password for DIDStore.
 * @param
 *      old                     [in] Old store password for DIDStore.
 * @return
 *      0 on success, -1 if an error occurred. Caller should free the returned value.
 */
DID_API int DIDStore_ChangePassword(DIDStore *store, const char *new, const char *old);

/**
 * \~English
 * Export DID information into file with json format. The json content include document,
 * credentials, private keys and meta.
 *
 * @param
 *      store                   [in] The handle to DIDStore.
 * @param
 *      storepass               [in] Password for DIDStore.
 * @param
 *      did                     [in] The handle to DID.
 * @param
 *      file                    [in] Export file.
 * @param
 *      password                [in] Password to encrypt.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_ExportDID(DIDStore *store, const char *storepass, DID *did,
        const char *file, const char *password);

/**
 * \~English
 * Import DID information by file.
 *
 * @param
 *      store                   [in] The handle to DIDStore.
 * @param
 *      storepass               [in] Password for DIDStore.
 * @param
 *      file                    [in] Export file.
 * @param
 *      password                [in] Password to encrypt.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_ImportDID(DIDStore *store, const char *storepass,
        const char *file, const char *password);

/**
 * \~English
 * Export private identity information into file with json format.
 * The json content include mnemonic(encrypted), extended private key(encrypted),
 * extended public key(if has it, dont't encrypted) and index.
 *
 * @param
 *      store                   [in] The handle to DIDStore.
 * @param
 *      storepass               [in] Password for DIDStore.
 * @param
 *      file                    [in] Export file.
 * @param
 *      password                [in] Password to encrypt.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_ExportPrivateIdentity(DIDStore *store, const char *storepass,
        const char *file, const char *password);

/**
 * \~English
 * Import private identity by file.
 *
 * @param
 *      store                   [in] The handle to DIDStore.
 * @param
 *      storepass               [in] Password for DIDStore.
 * @param
 *      did                     [in] The handle to DID.
 * @param
 *      file                    [in] Export file.
 * @param
 *      password                [in] Password to encrypt.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_ImportPrivateIdentity(DIDStore *store, const char *storepass,
        const char *file, const char *password);
/**
 * \~English
 * Export whole store information into zip file.
 *
 * @param
 *      store                   [in] The handle to DIDStore.
 * @param
 *      storepass               [in] Password for DIDStore.
 * @param
 *      zipfile                 [in] Zip file to export.
 * @param
 *      password                [in] Password to encrypt.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_ExportStore(DIDStore *store, const char *storepass,
        const char *zipfile, const char *password);

/**
 * \~English
 * Import zip file into new DIDStore.
 *
 * @param
 *      store                   [in] The handle to DIDStore.
 * @param
 *      storepass               [in] Password for DIDStore.
 * @param
 *      zipfile                 [in] zip file to import.
 * @param
 *      password                [in] Password to encrypt.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_ImportStore(DIDStore *store, const char *storepass,
        const char *zipfile, const char *password);

/******************************************************************************
 * Mnemonic
 *****************************************************************************/
/**
 * \~English
 * Gernerate a random mnemonic.
 *
 * @param
 *      language               [in] The language for DID.
 *                             support language string: "chinese_simplified",
 *                             "chinese_traditional", "czech", "english", "french",
 *                             "italian", "japanese", "korean", "spanish".
 * @return
 *      mnemonic string. Use Mnemonic_free after finish using mnemonic string.
 */
DID_API const char *Mnemonic_Generate(const char *language);

/**
 * \~English
 * Free mnemonic buffer.
 *
 * @param
 *      mnemonic               [in] mnemonic buffter.
 */
DID_API void Mnemonic_Free(void *mnemonic);

/**
 * \~English
 * Check mnemonic.
 *
 * @param
 *      mnemonic               [in] mnemonic buffter.
 * @param
 *      language               [in] The language for DID.
 *                             0: English; 1: French; 2: Spanish;
 *                             3: Chinese_simplified;
 *                             4: Chinese_traditional;
 *                             5: Japanese.
 * @return
 *      true, if mnemonic is valid. or else, return false.
 */
DID_API bool Mnemonic_IsValid(const char *mnemonic, const char *language);

/******************************************************************************
 * Presentation
 *****************************************************************************/
/**
 * \~English
 * Create a presentation including some credentials.
 *
 * @param
 *      did                      [in] The handle to DID.
 * @param
 *      signkey                  [in] The key id to sign.
 * @param
 *      store                    [in] The handle to DIDStore.
 * @param
 *      storepass                [in] The password of DIDStore.
 * @param
 *      nonce                    [in] Indicate the usage of Presentation.
  * @param
 *      realm                    [in] Indicate where the Presentation is use.
 * @param
 *      count                    [in] The count of Credentials.
 * @return
 *      If no error occurs, return the handle to Presentataion.
 *      Otherwise, return NULL.
 *      Notice that user need to release the handle of returned instance to destroy it's memory.
 */
DID_API Presentation *Presentation_Create(DID *did, DIDURL *signkey, DIDStore *store,
        const char *storepass, const char *nonce, const char *realm, int count, ...);

/**
 * \~English
 * Destroy Presentation.
 *
 * @param
 *      pre                      [in] The handle to Presentation.
 */
DID_API void Presentation_Destroy(Presentation *pre);

/**
 * \~English
 * Get json context from Presentation.
 *
 * @param
 *      pre                  [in] A handle to Presentation.
 * @param
 *      normalized           [in] Json context is normalized or not.
 *                           true represents normalized, false represents not normalized.
 * @return
 *      If no error occurs, return json context. Otherwise, return NULL.
 *      Notice that user need to free the returned value that it's memory.
 */
DID_API const char* Presentation_ToJson(Presentation *pre, bool normalized);

/**
 * \~English
 * Get Presentation from json context.
 *
 * @param
 *      json                 [in] Json context about Presentation.
 * @return
 *      If no error occurs, return the handle to Presentation.
 *      Otherwise, return NULL.
 *      Notice that user need to release the handle of returned instance to destroy it's memory.
 */
DID_API Presentation *Presentation_FromJson(const char *json);

/**
 * \~English
 * Get the DID for signing the Presentation.
 *
 * @param
 *      pre                   [in] The handle to Presentation.
 * @return
 *      If no error occurs, return the handle to DID.
 *      Otherwise, return NULL.
 */
DID_API DID *Presentation_GetSigner(Presentation *pre);

/**
 * \~English
 * Get Credential count in Presentation.
 *
 * @param
 *      pre                   [in] The handle to Presentation.
 * @return
 *      If no error occurs, return the count of Credential.
 *      Otherwise, return -1.
 */
DID_API ssize_t Presentation_GetCredentialCount(Presentation *pre);

/**
 * \~English
 * Get Credential list for signing the Presentation.
 *
 * @param
 *      pre                   [in] The handle to Presentation.
 * @param
 *      creds                 [out] The buffer that will receive the public keys.
  * @param
 *      size                  [in] The count of Credentials.
 * @return
 *      If no error occurs, return the count of Credential.
 *      Otherwise, return -1.
 */
DID_API ssize_t Presentation_GetCredentials(Presentation *pre,
        Credential **creds, size_t size);

/**
 * \~English
 * Get Credential list for signing the Presentation.
 *
 * @param
 *      pre                   [in] The handle to Presentation.
 * @param
 *      credid                [in] The Credential Id.
 * @return
 *      If no error occurs, return the handle to Credential.
 *      Otherwise, return NULL.
 */
DID_API Credential *Presentation_GetCredential(Presentation *pre, DIDURL *credid);

/**
 * \~English
 * Get Presentation Type.
 *
 * @param
 *      pre                   [in] The handle to Presentation.
 * @return
 *      If no error occurs, return the Presentation Type string.
 *      Otherwise, return NULL.
 */
DID_API const char *Presentation_GetType(Presentation *pre);

/**
 * \~English
 * Get time created Presentation.
 *
 * @param
 *      pre                   [in] The handle to Presentation.
 * @return
 *      If no error occurs, return the time created Presentation.
 *      Otherwise, return 0.
 */
DID_API time_t Presentation_GetCreatedTime(Presentation *pre);

/**
 * \~English
 * Get key to sign Presentation.
 *
 * @param
 *      pre                   [in] The handle to Presentation.
 * @return
 *      If no error occurs, return the handle to sign key.
 *      Otherwise, return NULL.
 */
DID_API DIDURL *Presentation_GetVerificationMethod(Presentation *pre);

/**
 * \~English
 * Get Presentation nonce.
 *
 * @param
 *      pre                   [in] The handle to Presentation.
 * @return
 *      If no error occurs, return the Presentaton nonce string.
 *      Otherwise, return NULL.
 */
DID_API const char *Presentation_GetNonce(Presentation *pre);

/**
 * \~English
 * Get Presentation realm.
 *
 * @param
 *      pre                   [in] The handle to Presentation.
 * @return
 *      If no error occurs, return the Presentaton realm string.
 *      Otherwise, return NULL.
 */
DID_API const char *Presentation_GetRealm(Presentation *pre);

/**
 * \~English
 * Presentation is genuine or not.
 *
 * @param
 *      pre                      [in] The Presentation handle.
 * @return
 *      flase if not genuine, true if genuine.
 */
DID_API bool Presentation_IsGenuine(Presentation *pre);

/**
 * \~English
 * Presentation is valid or not.
 *
 * @param
 *      pre              [in] The Presentation handle.
 * @return
 *      flase if not valid, true if valid.
 */
DID_API bool Presentation_IsValid(Presentation *pre);

/**
 * \~English
 * Initialize DIDBackend to resolve by url.
 *
 * @param
 *      url              [in] The URL string.
 * @param
 *      cachedir         [in] The directory for cache.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDBackend_InitializeDefault(const char *url, const char *cachedir);

/**
 * \~English
 * Initialize DIDBackend to resolve by DIDResolver.
 *
 * @param
 *      resolver          [in] The handle to DIDResolver.
 * @param
 *      cachedir          [in] The directory for cache.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDBackend_Initialize(DIDResolver *resolver, const char *cachedir);

/**
 * \~English
 * Set ttl for resolve cache.
 *
 * @param
 *      ttl            [in] The time for cache.
 */
DID_API void DIDBackend_SetTTL(long ttl);

/**
 * \~English
 * User set DID Local Resolve handle in order to give which did document to verify.
 * If handle != NULL, set DID Local Resolve Handle; If handle == NULL, clear this handle.
 *
 * @param
 *      handle            [in] The pointer to DIDLocalResovleHandle function.
 */
DID_API void DIDBackend_SetLocalResolveHandle(DIDLocalResovleHandle *handle);

/******************************************************************************
 * Error handling
 *****************************************************************************/

#define DIDSUCCESS                                  0
/**
 * \~English
 * Argument(s) is(are) invalid.
 */
#define DIDERR_INVALID_ARGS                         0x8D000001
/**
 * \~English
 * Runs out of memory.
 */
#define DIDERR_OUT_OF_MEMORY                        0x8D000002
/**
 * \~English
 * IO error.
 */
#define DIDERR_IO_ERROR                             0x8D000003
/**
 * \~English
 * DID is malformed.
 */
#define DIDERR_MALFORMED_DID                        0x8D000004
/**
 * \~English
 * DIDURL is malformed.
 */
#define DIDERR_MALFORMED_DIDURL                     0x8D000005
/**
 * \~English
 * DIDDocument is malformed.
 */
#define DIDERR_MALFORMED_DOCUMENT                   0x8D000006
/**
 * \~English
 * Credential is malformed.
 */
#define DIDERR_MALFORMED_CREDENTIAL                 0x8D000007
/**
 * \~English
 * Presentation is malformed.
 */
#define DIDERR_MALFORMED_PRESENTATION               0x8D000008
/**
 * \~English
 * Meta(DIDMetaData/CredMetaData) is malformed.
 */
#define DIDERR_MALFORMED_META                       0x8D000009
/**
 * \~English
 * DID object already exists.
 */
#define DIDERR_ALREADY_EXISTS                       0x8D00000A
/**
 * \~English
 * DID object doesn't already exists.
 */
#define DIDERR_NOT_EXISTS                           0x8D00000B
/**
 * \~English
 * DID is expired.
 */
#define DIDERR_EXPIRED                              0x8D00000C
/**
 * \~English
 * DID is deactivated.
 */
#define DIDERR_DID_DEACTIVATED                      0x8D00000D
/**
 * \~English
 * DID is not genuine.
 */
#define DIDERR_NOT_GENUINE                          0x8D00000E
/**
 * \~English
 * Crypto failed.
 */
#define DIDERR_CRYPTO_ERROR                         0x8D00000F
/**
 * \~English
 * Error from DIDStore.
 */
#define DIDERR_DIDSTORE_ERROR                       0x8D000010
/**
 * \~English
 * key is invalid.
 */
#define DIDERR_INVALID_KEY                          0x8D000011
/**
 * \~English
 * No valid backend to resolve.
 */
#define DIDERR_INVALID_BACKEND                      0x8D000012
/**
 * \~English
 * Resolve DID info failed.
 */
#define DIDERR_RESOLVE_ERROR                        0x8D000013
/**
 * \~English
 * Resolve result is malformed.
 */
#define DIDERR_MALFORMED_RESOLVE_RESULT             0x8D000014
/**
 * \~English
 * Error from id transaction.
 */
#define DIDERR_TRANSACTION_ERROR                    0x8D000015
/**
 * \~English
 * Unsupported error.
 */
#define DIDERR_UNSUPPOTED                           0x8D000016
/**
 * \~English
 * JWT error.
 */
#define DIDERR_JWT                                  0x8D000017
/**
 * \~English
 * JWT error.
 */
#define DIDERR_MALFORMED_EXPORTDID                  0x8D000018
/**
 * \~English
 * Unknown error.
 */
#define DIDERR_UNKNOWN                              0x8D0000FF

/**
 * \~English
 * Get the last error code.
 */
#define DIDERRCODE                                 (DIDError_GetCode())
/**
 * \~English
 * Get the last error message.
 */
#define DIDERRMSG                                  (DIDError_GetMessage())
/**
 * \~English
 * Get file about the last error.
 */
#define DIDERRFILE                                 (DIDError_GetFile())
/**
 * \~English
 * Get line about the last error.
 */
#define DIDERRLINE                                 (DIDError_GetLine())

/**
 * \~English
 * Clear the last-error code.
 */
DID_API void DIDError_Clear(void);
/**
 * \~English
 * Print the whole information of last-error code.
 */
DID_API void DIDError_Print(void);

/**
 * \~English
 * Get the last-error code.
 */
DID_API int DIDError_GetCode(void);
/**
 * \~English
 * Get the last-error message.
 */
DID_API const char *DIDError_GetMessage(void);

/**
 * \~English
 * Get the file for last-error.
 */
DID_API const char *DIDError_GetFile(void);
/**
 * \~English
 * Get line for last-error.
 */
DID_API int DIDError_GetLine(void);


#ifdef __cplusplus
}
#endif

#endif /* __ELA_DID_H__ */
