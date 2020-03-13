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

typedef enum {
    DID_FILTER_ALL,
    DID_FILTER_HAS_PRIVATEKEY,
    DID_FILTER_NO_PRIVATEKEY
} ELA_DID_FILTER;

/**
 * \~English
 * The value of the credential Subject property is defined as
 * a set of objects that contain one or more properties that are
 * each related to a subject of the credential.
 */
typedef struct Property {
    char *key;
    char *value;
} Property;

/**
 * \~English
 * DID is a globally unique identifier that does not require
 * a centralized registration authority.
 * It includes method specific string. (elastos:id:ixxxxxxxxxx).
 */
typedef struct DID                 DID;
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

struct DIDAdapter {
    const char* (*createIdTransaction) (DIDAdapter *adapter,
            const char *payload, const char *memo);
};

struct DIDResolver {
    const char* (*resolve) (DIDResolver *resolver, const char *did, int all);
};

/**
 * \~English
 * DID list callbacks, return alias about did.
 */
typedef int DIDStore_DIDsCallback(DID *did, void *context);
/**
 * \~English
 * Credential list callbacks, return alias about credential.
 */
typedef int DIDStore_CredentialsCallback(DIDURL *id, void *context);

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
 * Copy one DID to the other DID.
 *
 * @param
 *      dest                   [in] DID to be copied.
 * @param
 *      src                   [in] DID be copied.
 * @return
 *      the handle to dest DID if succuss, NULL if failed .
 */
DID_API DID *DID_Copy(DID *dest, DID *src);

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
 * @ User need to destroy DID Document.
 */
DID_API DIDDocument *DID_Resolve(DID *did, bool force);

/**
 * \~English
 * Get all DID Documents from chain.
 *
 * @param
 *      did                      [in] The handle of DID.
 * @return
 *      If no error occurs, return the handle to DID Document buffter. Free
 *      Otherwise, return NULL.
 * @ User need to free the return value and destroy every DID Document.
 */
DID_API DIDDocument **DID_ResolveAll(DID *did);

/**
 * \~English
 * Set alias for did.
 *
 * @param
 *      did                      [in] The handle of DID.
  * @param
 *      alias                    [in] Alias string for DID.
 * @return
 *      If no error occurs, return 0.
 *      Otherwise, return -1.
 */
DID_API int DID_SetAlias(DID *did, const char *alias);

/**
 * \~English
 * Get alias for did.
 *
 * @param
 *      did                        [in] The handle of DID.
 * @return
 *      If no error occurs, return alias string.
 *      Otherwise, return NULL.
 */
DID_API const char *DID_GetAlias(DID *did);

/**
 * \~English
 * Get transaction id of did.
 *
 * @param
 *      did                         [in] The handle of DID.
 * @return
 *      If no error occurs, return transaction string.
 *      Otherwise, return NULL.
 */
DID_API const char *DID_GetTxid(DID *did);

/**
 * \~English
 * Get did status, deactived or not.
 *
 * @param
 *      did                      [in] The handle of DID.
 * @return
 *      If no error occurs, return status.
 *      Otherwise, return false.
 */
DID_API bool DID_GetDeactived(DID *did);

/**
 * \~English
 * Get the last timestamp of transaction id for did.
 *
 * @param
 *      did                      [in] The handle of DID.
 * @return
 *      If no error occurs, return time stamp.
 *      Otherwise, return 0.
 */
DID_API time_t DID_GetLastTransactionTimestamp(DID *did);
/******************************************************************************
 * DIDURL
 *****************************************************************************/
/**
 * \~English
 * Get DID URL from string.
 *
 * @param
 *      idstring     [in] A pointer to string including id information.
 *                   idstring support: 1. did:elastos:xxxxxxx#xxxxx
 *                                     2. #xxxxxxx
 * @param
 *      ref          [in] A pointer to DID.
 * @return
 *      If no error occurs, return the handle to DID.
 *      Otherwise, return NULL.
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
 *      If no error occurs, return id string.
 *      Otherwise, return NULL.
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
 * Copy one DID URL to the other DID URL.
 *
 * @param
 *      dest                [in] DID URL to be copied.
 * @param
 *      src                 [in] DID URL be copied.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API DIDURL *DIDURL_Copy(DIDURL *dest, DIDURL *src);

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
 * Set alias for id.
 *
 * @param
 *      id                       [in] The handle of DIDURL.
  * @param
 *      alias                    [in] Alias string for DID.
 * @return
 *      If no error occurs, return 0.
 *      Otherwise, return -1.
 */
DID_API int DIDURL_SetAlias(DIDURL *id, const char *alias);

/**
 * \~English
 * Get alias for id, mainly for credential.
 *
 * @param
 *      id                        [in] The handle of DIDURL.
 * @return
 *      If no error occurs, return alias string.
 *      Otherwise, return NULL.
 */
DID_API const char *DIDURL_GetAlias(DIDURL *id);

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
 */
DID_API DIDDocument *DIDDocument_FromJson(const char* json);

/**
 * \~English
 * Get json context from DID Document.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      normalized           [in] Json context is normalized or not.
 *                           true represents normalized, false represents not compact.
 * @return
 *      If no error occurs, return json context. Return value must be free after
 *      finishing use.
 *      Otherwise, return NULL.
 */
DID_API const char *DIDDocument_ToJson(DIDDocument *document, bool normalized);

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
 *      If no error occurs, return a handle to DID.
 *      Otherwise, return NULL.
 */
DID_API DIDDocumentBuilder* DIDDocument_Edit(DIDDocument *document);

/**
 * \~English
 * Destroy DIDDocument Builder.
 *
 * @param
 *      builder             [in] A handle to DIDDocument Builder.
 * @return
 *      If no error occurs, return a handle to DID.
 *      Otherwise, return NULL.
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
 *      key                  [in] An identifier of public key.
 * @param
 *      controller           [in] A controller property, identifies
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
 *      size                 [in] The size of credential subject property.
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
 *      point                [in] ServiceEndpoint property is a valid URI.
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
 *      document                [in] The handle to DID Document.
 * @param
 *      keyid                    [in] Public key to sign.
 *                                   If key = NULL, sdk will get default key from
 *                                   DID Document.
 * @param
 *      storepass               [in] Pass word to sign.
 * @param
 *      sig                     [out] The buffer will receive signature data.
 * @param
 *      count                   [in] The size of data list.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocument_Sign(DIDDocument *document, DIDURL *keyid, const char *storepass,
        char *sig, int count, ...);

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
 * Set nickname for DID.
 *
 * @param
 *      document                 [in] The handle to DIDDocument.
 * @param
 *      alias                    [in] The nickname to store.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocument_SetAlias(DIDDocument *document, const char *alias);

/**
 * \~English
 * Get nickname for DID.
 *
 * @param
 *      document                    [in] The handle to DIDDocument.
 * @return
 *      If no error occurs, return alias string.
 *      Otherwise, return NULL.
 */
DID_API const char *DIDDocument_GetAlias(DIDDocument *document);

/**
 * \~English
 * Get transaction id of the latest updated DID Document.
 *
 * @param
 *      document                     [in] The handle to DIDDocument.
 * @return
 *      If no error occurs, return 0. Otherwise, return -1.
 */
DID_API const char *DIDDocument_GetTxid(DIDDocument *document);

/**
 * \~English
 * Get timestamp of the latest transaction.
 *
 * @param
 *      document                     [in] The handle to DID Document.
 * @return
 *      If no error occurs, return timestamp.
 *      Otherwise, return NULL.
 */
DID_API time_t DIDDocument_GetLastTransactionTimestamp(DIDDocument *document);

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
 * Get json context from Credential.
 *
 * @param
 *      cred                 [in] A handle to Credential.
 * @param
 *      normalized           [in] Json context is normalized or not.
 *                           true represents normalized, false represents not.
 * @return
 *      If no error occurs, return json context. Return value must be free after
 *      finishing use.
 *      Otherwise, return NULL.
 */
DID_API const char* Credential_ToJson(Credential *cred, bool normalized);

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
 * @param
 *      properties           [out] The buffer that will receive
 *                                 credential subject properties.
 * @param
 *      size                 [in] The buffer size of credential subject properties.
 * @return
 *      size of subject porperties on success, -1 if an error occurred.
 */
DID_API ssize_t Credential_GetProperties(Credential *cred, Property *properties, size_t size);

/**
 * \~English
 * Get specified subject property according to the key of property.
 *
 * @param
 *      cred                 [in] A handle to Credential.
 * @param
 *      name                 [in] The key of property.
 * @return
 *      If no error occurs, return property string.
 *      Otherwise, return NULL.
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
 * Verify the credential is valid or not.
 * Issuance always occurs before any other actions involving a credential.
 *
 * @param
 *      cred                     [in] The Credential handle.
 * @return
 *      0 on success, -1 if an error occurred or verify failed.
 */
DID_API int Credential_Verify(Credential *cred);

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
DID_API int Credential_SetAlias(Credential *credential, const char *alias);

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
DID_API const char *Credential_GetAlias(Credential *credential);

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
 * @return
 *      The handle of Issuer.
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
 * An issuer issues a verifiable credential to a holder.
 * Issuance always occurs before any other actions involving a credential.
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
 *      properties           [in] The array of credential subject property.
 * @param
 *      size                 [in] The size of credential subject property.
 * @param
 *      expires              [in] The time to credential be expired.
 * @param
 *      passphase            [in] Pass word to sign.
 * @return
 *      If no error occurs, return the handle to Credential issued.
 *      Otherwise, return NULL.
 */
DID_API Credential *Issuer_CreateCredential(Issuer *issuer, DID *owner, DIDURL *credid,
        const char **types, size_t typesize, Property *properties, int size,
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
 * Store seed of keypair to initial user's identity.
 *
  * @param
 *      store                  [in] THe handle to DIDStore.
 * @param
 *      storepass              [in] The password for DIDStore.
 * @param
 *      mnemonic               [in] Mnemonic for generate key.
 * @param
 *      passphrase             [in] The pass word to generate private identity.
 * @param
 *      language               [in] The language for DID.
 *                             support language string: "chinese_simplified",
 *                             "chinese_traditional", "czech", "english", "french",
 *                             "italian", "japanese", "korean", "spanish".
 * @param
 *      extendedkey            [in] Extendedkey string.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_InitPrivateIdentity(DIDStore *store, const char *storepass,
        const char *mnemonic, const char *passphrase, const char *language, bool force);

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
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_Synchronize(DIDStore *store, const char *storepass);

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
 * @return
 *      If no error occurs, return the handle to DID Document.
 *      Otherwise, return NULL.
 */
DID_API DIDDocument *DIDStore_NewDID(DIDStore *store, const char *storepass,
        const char *alias);

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
 * Sign data by DID.
 *
 * @param
 *      store                   [in] The handle to DIDStore.
 * @param
 *      storepass               [in] Password for DIDStore.
 * @param
 *      did                     [in] The handle to DID.
 * @param
 *      key                     [in] Public key to sign.
 * @param
 *      sig                     [out] The buffer will receive signature data.
 * @param
 *      count                   [in] The size of data list.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_Sign(DIDStore *store, const char *storepass, DID *did,
        DIDURL *key, char *sig, int count, ...);

DID_API int DIDStore_Signv(DIDStore *store, const char *storepass, DID *did,
        DIDURL *key, char *sig, int count, va_list inputs);

/**
 * \~English
 * Store DID Document in DID Store.
 *
 * @param
 *      store                     [in] The handle to DIDStore.
 * @param
 *      document                  [in] The handle to DID Document.
 * @param
 *      alias                     [in] The nickname of DID.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_StoreDID(DIDStore *store, DIDDocument *document, const char *alias);

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
 * @param
 *      alias                    [in] The nickname of credential.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_StoreCredential(DIDStore *store, Credential *credential,
        const char *alias);

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
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_StorePrivateKey(DIDStore *store, const char *storepass,
        DID *did, DIDURL *id, const uint8_t *privatekey);

/**
 * \~English
 * Delete private key.
 *
 * @param
 *      store                   [in] The handle to DIDStore.
 * @param
 *      did                     [in] The handle to DID.
 * @param
 *      id                      [in] The identifier of public key.
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
 *      signKey                  [in] The public key to sign.
 * @param
 *      force                    [in] Force document into chain.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API const char *DIDStore_PublishDID(DIDStore *store, const char *storepass,
        DID *did, DIDURL *signKey, bool force);

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
 *      signKey                  [in] The public key to sign.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API const char *DIDStore_DeactivateDID(DIDStore *store, const char *storepass,
        DID *did, DIDURL *signKey);

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
 * Verify Presentation is valid or not.
 *
 * @param
 *      pre                      [in] The handle to Presentation.
 */
DID_API int Presentation_Verify(Presentation *pre);

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
 *      If no error occurs, return json context. Return value must be free after
 *      finishing use.
 *      Otherwise, return NULL.
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
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDBackend_InitializeDefault(const char *url, const char *cachedir);

/**
 * \~English
 * Initialize DIDBackend to resolve by DIDResolver.
 *
 * @param
 *      resolver            [in] The handle to DIDResolver.
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

#ifdef __cplusplus
}
#endif

#endif /* __ELA_DID_H__ */
