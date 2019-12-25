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
 * DID is a globally unique identifier that does not require
 * a centralized registration authority.
 * It includes method specific string. (elastos:id:ixxxxxxxxxx).
 */
typedef struct DID              DID;
/**
 * \~English
 * DID URL defines by the did-url rule, refers to a URL that begins with a DID
 * followed by one or more additional components. A DID URL always
 * identifies the resource to be located.
 * DIDURL includes DID and Url fragment by user defined.
 */
typedef struct DIDURL           DIDURL;
/**
 * \~English
 * Public keys are used for digital signatures, encryption and
 * other cryptographic operations, which are the basis for purposes such as
 * authentication or establishing secure communication with service endpoints.
 */
typedef struct PublicKey        PublicKey;
/**
 * \~English
 * Credential is a set of one or more claims made by the same entity.
 * Credentials might also include an identifier and metadata to
 * describe properties of the credential.
 */
typedef struct Credential       Credential;
/**
 * \~English
 * A Presentation can be targeted to a specific verifier by using a Linked Data
 * Proof that includes a nonce and realm.
 * This also helps prevent a verifier from reusing a verifiable presentation as
 * their own.
 */
typedef struct Presentation     Presentation;
/**
 * \~English
 * A service endpoint may represent any type of service the subject
 * wishes to advertise, including decentralized identity management services
 * for further discovery, authentication, authorization, or interaction.
 */
typedef struct Service          Service;
/**
 * \~English
 * A DID resolves to a DID Document. This is the concrete serialization of
 * the data model, according to a particular syntax.
 * DIDDocument is a set of data that describes the subject of a DID,
 * including public key, authentication(optional), authorization(optional),
 * credential and services. One document must be have only subject,
 * and at least one public key.
 */
typedef struct DIDDocument      DIDDocument;
/**
 * \~English
 * A issuer is the did to issue credential. Issuer includes issuer's did and
 * issuer's sign key.
 */
typedef struct Issuer           Issuer;
/**
 * \~English
 * DIDEntry includes did(DID) and hint for list did callback.
 */
typedef struct DIDEntry         DIDEntry;
/**
 * \~English
 * CredentialEntry includes id(DIDURL) and hint for list credential callback.
 */
typedef struct CredentialEntry  CredentialEntry;
/**
 * \~English
 * The value of the credential Subject property is defined as
 * a set of objects that contain one or more properties that are
 * each related to a subject of the credential.
 */
typedef struct Property         Property;
/**
 * \~English
 * DIDStore is local store for specified DID.
 */
typedef struct DIDStore         DIDStore;
/**
 * \~English
 * DIDAdapter is use for various method to create did transaction and resolve
 * did document from chain.
 */
typedef struct DIDAdapter      DIDAdapter;

struct DIDAdapter {
    const char* (*createIdTransaction) (DIDAdapter *adapter, const char *payload, const char *memo);
    const char* (*resolve) (DIDAdapter *adapter, const char *did);
};
/**
 * \~English
 * DID list callbacks, return hint about did.
 * API need to free entry memory.
 */
typedef int DIDStore_GetDIDHintCallback(DIDEntry *entry, void *context);
/**
 * \~English
 * Credential list callbacks, return hint about credential.
 * API need to free entry memory.
 */
typedef int DIDStore_GetCredHintCallback(CredentialEntry *entry, void *context);

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
 *      newdid                   [in] DID to be copied.
 * @param
 *      olddid                   [in] DID be copied.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DID_Copy(DID *newdid, DID *olddid);

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
 * Get DID Document from chain.
 *
 * @param
 *      did                      [in] The handle of DID.
 * @return
 *      If no error occurs, return the handle to DID Document.
 *      Otherwise, return NULL.
 */
DID_API DIDDocument *DID_Resolve(DID *did);

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
 *      newid                [in] DID URL to be copied.
 * @param
 *      oldid                [in] DID URL be copied.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDURL_Copy(DIDURL *newid, DIDURL *oldid);

/**
 * \~English
 * Destroy DID URL.
 *
 * @param
 *      id                  [in] A handle to DID URL to be destroied.
 */
DID_API void DIDURL_Destroy(DIDURL *id);

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
 *      compact              [in] Json context is compact or not.
 *                           1 represents compact, 0 represents not compact.
 * @param
 *      forsign              [in] Json context needs to sign or not.
 *                           1 represents forsign, 0 represents not forsign.
 * @return
 *      If no error occurs, return json context. Return value must be free after
 *      finishing use.
 *      Otherwise, return NULL.
 */
DID_API const char *DIDDocument_ToJson(DIDDocument *document, int compact, int forsign);

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
 * Add public key to DID Document.
 * Each public key has an identifier (id) of its own, a type, and a controller,
 * as well as other properties publicKeyBase58 depend on which depend on
 * what type of key it is.
 *
 * @param
 *      document             [in] A handle to DID Document.
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
DID_API int DIDDocument_AddPublicKey(DIDDocument *document, DIDURL *keyid,
        DID *controller, const char *key);

/**
 * \~English
 * Remove specified public key from DID Document.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      key                  [in] An identifier of public key.
 * @param
 *      force                [in] An identifier of public key.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocument_RemovePublicKey(DIDDocument *document, DIDURL *keyid, bool force);

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
 *      id                   [in] An identifier of public key.
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
 *      id                   [in] An identifier of public key to be selected.
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
 * Add public key to Authenticate.
 * Authentication is the mechanism by which the controller(s) of a DID can
 * cryptographically prove that they are associated with that DID.
 * A DID Document must include an authentication property.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      key                  [in] An identifier of public key.
 * @param
 *      controller           [in] A controller property, identifies
 *                              the controller of the corresponding private key.
 * @param
 *      key                  [in] Key property depend on key type.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocument_AddAuthenticationKey(DIDDocument *document, DIDURL *keyid,
        const char *key);

/**
 * \~English
 * Remove authentication key from Authenticate.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      key                  [in] An identifier of public key.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocument_RemoveAuthenticationKey(DIDDocument *document, DIDURL *keyid);

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
 *      id                   [in] An identifier of authentication key.
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
 *      id                   [in] An identifier of authentication key to be selected.
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
 * Add public key to authorizate.
 * Authorization is the mechanism used to state
 * how operations may be performed on behalf of the DID subject.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      key                  [in] An identifier of authorization key.
 * @param
 *      controller           [in] A controller property, identifies
 *                              the controller of the corresponding private key.
 * @param
 *      key                  [in] Key property depend on key type.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocument_AddAuthorizationKey(DIDDocument *document, DIDURL *keyid,
        DID *controller, const char *key);

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
 * Add Authorization key to Authentication array according to DID.
 * Authentication is the mechanism by which the controller(s) of a DID can
 * cryptographically prove that they are associated with that DID.
 * A DID Document must include an authentication property.
 *
 * @param
 *      controller           [in] A controller property, identifies
 *                              the controller of the corresponding private key.
 * @param
 *      keyid                [in] An identifier of public key.
 * @param
 *      key                  [in] Key property depend on key type.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocument_AuthorizationDid(DIDDocument *document, DIDURL *keyid,
        DID *controller, DIDURL *authorkeyid);

/**
 * \~English
 * Remove authorization key from authorizate.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      key                  [in] An identifier of authorization key.
 * @param
 *      controller           [in] A controller property, identifies
 *                              the controller of the corresponding private key.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocument_RemoveAuthorizationKey(DIDDocument *document, DIDURL *keyid);

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
 *      id                   [in] An identifier of authorization key.
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
 *      id                   [in] An identifier of authorization key to be selected.
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
 * Add one credential to credential array.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      credential           [in] The handle to Credential.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocument_AddCredential(DIDDocument *document, Credential *credential);

/**
 * \~English
 * Remove specified credential from credential array.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      credential           [in] The handle to Credential.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocument_RemoveCredential(DIDDocument *document, DIDURL *credid);
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
 *      id                   [in] An identifier of Credential.
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
 *      id                   [in] An identifier of Credential to be selected.
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
 * Add one Service to services array.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      id                   [in] The identifier of Service.
 * @param
 *      type                 [in] The type of Service.
 * @param
 *      point                [in] ServiceEndpoint property is a valid URI.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocument_AddService(DIDDocument *document, DIDURL *serviceid,
        const char *type, const char *point);

/**
 * \~English
 * Remove specified Service to services array.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      id                   [in] The identifier of Service.
 * @param
 *      type                 [in] The type of Service.
 * @param
 *      point                [in] ServiceEndpoint property is a valid URI.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocument_RemoveService(DIDDocument *document, DIDURL *serviceid);

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
DID_API ssize_t DIDDocument_GetServices(DIDDocument *document, Service **services, size_t size);

/**
 * \~English
 * Get Service according to identifier of Service.
 *
 * @param
 *      document             [in] A handle to DID Document.
 * @param
 *      id                   [in] An identifier of Service.
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
 *      id                   [in] An identifier of Service to be selected.
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
 * Set expire time about DID Document.
 *
 * @param
 *      time             [in] time to expire.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDDocument_SetExpires(DIDDocument *document, time_t expires);

/**
 * \~English
 * Sign data by DID.
 *
 * @param
 *      document                [in] The handle to DID Document.
 * @param
 *      key                     [in] Public key to sign.
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
 *      key                     [in] Public key to sign.
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
 *      document             [in] A handle to Credential.
 * @param
 *      compact              [in] Json context is compact or not.
 *                           1 represents compact, 0 represents not compact.
 * @param
 *      forsign              [in] Json context needs to sign or not.
 *                           1 represents forsign, 0 represents not forsign.
 * @return
 *      If no error occurs, return json context. Return value must be free after
 *      finishing use.
 *      Otherwise, return NULL.
 */
DID_API const char* Credential_ToJson(Credential *cred, int compact, int forsign);

/**
 * \~English
 * Get one DID's Credential from json context.
 *
 * @param
 *      json                 [in] Json context about credential.
 * @param
 *      did                  [in] A handle to DID.
 * @return
 *      If no error occurs, return the handle to Credential.
 *      Otherwise, return NULL.
 */
DID_API Credential *Credential_FromJson(const char *json, DID *did);

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
 *      cred                 [in] The buffer size of credential subject properties.
 * @return
 *      size of subject porperties on success, -1 if an error occurred.
 */
DID_API ssize_t Credential_GetProperties(Credential *cred, Property **properties, size_t size);

/**
 * \~English
 * Get specified subject property according to the key of property.
 *
 * @param
 *      cred                 [in] A handle to Credential.
 * @param
 *      name                 [in] The key of property.
 * @return
 *      If no error occurs, return the handle to property.
 *      Otherwise, return NULL.
 */
DID_API Property *Credential_GetProperty(Credential *cred, const char *name);

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
 * Add specified type to Credential.
 *
 * @param
 *      cred                 [in] A handle to Credential.
 * @param
 *      type                 [in] Type to be added.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int Credential_AddType(Credential *cred, const char *type);

/**
 * \~English
 * Set the date of credential expired.
 *
 * @param
 *      cred                 [in] A handle to Credential.
 * @param
 *      time                 [in] The date of credential expired.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int Credential_SetExpirationDate(Credential *cred, time_t time);

/**
 * \~English
 * Add credential subject property to Credential.
 *
 * @param
 *      cred                 [in] A handle to Credential.
 * @param
 *      name                 [in] The key of property.
 * @param
 *      value                 [in] The value of property.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int Credential_AddProperty(Credential *cred, const char *name, const char *value);

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
DID_API Issuer *Issuer_Create(DID *did, DIDURL *signkey);

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
 *      did                  [in] A handle to DID.
 *                               The holder of this Credential.
 * @param
 *      fragment             [in] The portion of a DID URL.
 * @param
 *      issuer               [in] An issuer issues this credential.
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
DID_API Credential *Issuer_CreateCredential(DID *did, const char *fragment, Issuer *issuer,
        const char **types, size_t typesize, Property **properties, int size,
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
DID_API DIDStore* DIDStore_Initialize(const char *root, DIDAdapter *adapter);

/**
 * \~English
 * Get DIDStore handle.
 *
 * @return
 *      the handle.
 */
DID_API DIDStore* DIDStore_GetInstance(void);

/**
 * \~English
 * Deinitialize DIDStore.
 */
DID_API void DIDStore_Deinitialize(void);

/**
 * \~English
 * Check if it has private identity or not.
 *
 * @param
 *      store                 [in] The handle to DIDStore.
 * @return
 *      ture if it has identity, false if it has not.
 */
DID_API bool DIDStore_HasPrivateIdentity(DIDStore *store);

/**
 * \~English
 * Store seed of keypair to initial user's identity.
 *
  * @param
 *      store                  [in] THe handle to DIDStore.
 * @param
 *      mnemonic               [in] Mnemonic for generate key.
 * @param
 *      passphrase             [in] The pass word to generate private identity.
 * @param
 *      storepass              [in] The pass word of DID holder.
 * @param
 *      language               [in] The language for DID.
 *                             0: English; 1: French; 2: Spanish;
 *                             3: Japanese; 4: Chinese_simplified;
 *                             5: Chinese_traditional;
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_InitPrivateIdentity(DIDStore *store, const char *mnemonic,
        const char *passphrase, const char *storepass, const int language, bool force);

/**
 * \~English
 * Create new DID Document and store in the DID Store.
 *
 * @param
 *      storepass              [in] The pass word of DID holder.
 * @param
 *      hint                   [in] The nickname of DID.
 * @return
 *      If no error occurs, return the handle to DID Document.
 *      Otherwise, return NULL.
 */
DID_API DIDDocument *DIDStore_NewDID(DIDStore *store, const char *storepass, const char *hint);

/**
 * \~English
 * Sign data by DID.
 *
 * @param
 *      did                     [in] The handle to DID.
 * @param
 *      key                     [in] Public key to sign.
 * @param
 *      password                [in] Pass word to sign.
 * @param
 *      sig                     [out] The buffer will receive signature data.
 * @param
 *      count                   [in] The size of data list.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_Sign(DIDStore *store, DID *did, DIDURL *key,
        const char *storepass, char *sig, int count, ...);

DID_API int DIDStore_Signv(DIDStore *store, DID *did, DIDURL *key,
        const char *storepass, char *sig, int count, va_list inputs);

/**
 * \~English
 * Store DID Document in DID Store.
 *
 * @param
 *      document                 [in] The handle to DID Document.
 * @param
 *      hint                     [in] The nickname of DID.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_StoreDID(DIDStore *store, DIDDocument *document, const char *hint);

/**
 * \~English
 * Load DID Document from DID Store.
 *
 * @param
 *      did                     [in] The handle to DID.
 * @return
 *      If no error occurs, return the handle to DID Document.
 *      Otherwise, return NULL.
 */
DID_API DIDDocument *DIDStore_LoadDID(DIDStore *store, DID *did);

/**
 * \~English
 * Set nickname for DID.
 *
 * @param
 *      did                     [in] The handle to DID.
 * @param
 *      hint                    [in] The nickname to store.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_SetDIDHint(DIDStore *store, DID *did, const char *hint);

/**
 * \~English
 * Get nickname for DID.
 *
 * @param
 *      did                     [in] The handle to DID.
 * @return
 *      If no error occurs, return hint string. Return value must be free after
 *      finishing use.
 *      Otherwise, return NULL.
 */
DID_API const char *DIDStore_GetDIDHint(DIDStore *store, DID *did);

/**
 * \~English
 * Check if contain specific DID or not.
 *
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
 *      did                     [in] The handle to DID.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API void DIDStore_DeleteDID(DIDStore *store, DID *did);

/**
 * \~English
 * List DIDs in DID Store.
 *
 * @param
 *      callback                [in] Get DID hint call back.
 * @param
 *      context                 [in] The caller defined context data.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_ListDID(DIDStore *store, DIDStore_GetDIDHintCallback *callback,
        void *context);

/**
 * \~English
 * Store Credential in DID Store.
 *
 * @param
 *      credential              [in] The handle to Credential.
 * @param
 *      hint                    [in] The nickname of credential.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_StoreCredential(DIDStore *store, Credential *credential,
        const char *hint);

/**
 * \~English
 * Load Credential from DID Store.
 *
 * @param
 *      did                     [in] The handle to DID.
 * @param
 *      id                      [in] The identifier of credential.
 * @return
 *      If no error occurs, return the handle to Credential.
 *      Otherwise, return NULL.
 */
DID_API Credential *DIDStore_LoadCredential(DIDStore *store, DID *did, DIDURL *credid);

/**
 * \~English
 * Set Credential from DID Store.
 *
 * @param
 *      did                     [in] The handle to DID.
 * @param
 *      hint                    [in] The nickname of credential.
 * @return
*      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_SetCredentialHint(DIDStore *store, DID *did, DIDURL *credid,
        const char *hint);

/**
 * \~English
 * Get credential hint.
 *
 * @param
 *      did                     [in] The handle to DID.
 * @param
 *      id                      [in] The identifier of credential.
 * @return
 *      If no error occurs, return hint string. Return value must free after
 *      finishing use.
 *      Otherwise, return NULL.
 */
DID_API const char *DIDStore_GetCredentialHint(DIDStore *store, DID *did, DIDURL *credid);

/**
 * \~English
 * Check if contain any credential of specific DID.
 *
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
 *      did                     [in] The handle to DID.
 * @param
 *      id                      [in] The identifier of credential.
 * @return
 *      true on success, false if an error occurred.
 */
DID_API bool DIDStore_ContainsCredential(DIDStore *store, DID *did, DIDURL *credid);

/**
 * \~English
 * Delete specific credential.
 *
 * @param
 *      did                     [in] The handle to DID.
 * @param
 *      id                      [in] The identifier of credential.
 */
DID_API void DIDStore_DeleteCredential(DIDStore *store, DID *did, DIDURL *id);

/**
 * \~English
 * List credentials of specific DID.
 *
 * @param
 *      did                     [in] The handle to DID.
 * @param
 *      callback                [in] Get credential hint call back.
 * @param
 *      context                 [in] The caller defined context data.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_ListCredentials(DIDStore *store, DID *did,
        DIDStore_GetCredHintCallback *callback, void *context);

/**
 * \~English
 * Get credential conforming to identifier or type property.
 *
 * @param
 *      did                     [in] The handle to DID.
 * @param
 *      id                      [in] The identifier of credential.
 * @param
 *      type                    [in] The type of Credential to be selected.
 * @param
 *      callback                [in] Get credential hint call back.
 * @param
 *      context                 [in] The caller defined context data.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_SelectCredentials(DIDStore *store, DID *did, DIDURL *credid,
        const char *type, DIDStore_GetCredHintCallback *callback, void *context);

/**
 * \~English
 * Check if contain any private key of specific DID.
 *
 * @param
 *      did                     [in] The handle to DID.
 * @return
 *      true on success, false if an error occurred.
 */
DID_API bool DIDSotre_ContainPrivateKeys(DIDStore *store, DID *did);

/**
 * \~English
 * Check if contain specific private key of specific DID.
 *
 * @param
 *      did                     [in] The handle to DID.
 * @param
 *      id                      [in] The identifier of public key.
 * @return
 *      true on success, false if an error occurred.
 */
DID_API bool DIDStore_ContainPrivateKey(DIDStore *store, DID *did, DIDURL *keyid);

/**
 * \~English
 * Store private key.
 *
 * @param
 *      did                     [in] The handle to DID.
 * @param
 *      fragment                [in] The fragment of public key identifier.
  * @param
 *      privatekey              [in] Private key string.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int DIDStore_StorePrivateKey(DIDStore *store, DID *did, const char *fragment,
        const char *privatekey);

/**
 * \~English
 * Delete private key.
 *
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
 *      document                 [in] The handle to DID Document.
 * @param
 *      signKey                  [in] The public key to sign.
  * @param
 *      storepass                [in] Pass word to sign.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API const char *DIDStore_PublishDID(DIDStore *store, DIDDocument *document,
        DIDURL *signKey, const char *storepass);

/**
 * \~English
 * Update a DID Document on the chain.
 *
 * @param
 *      document                 [in] The handle to DID Document.
 * @param
 *      signKey                  [in] The public key to sign.
  * @param
 *      storepass                [in] Pass word to sign.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API const char *DIDStore_UpdateDID(DIDStore *store, DIDDocument *document,
        DIDURL *signKey, const char *storepass);

/**
 * \~English
 * Deactivate a DID on the chain.
 *
 * @param
 *      document                 [in] The handle to DID Document.
 * @param
 *      signKey                  [in] The public key to sign.
 * @param
 *      storepass                [in] Pass word to sign.
 * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API const char *DIDStore_DeactivateDID(DIDStore *store, DID *did,
        DIDURL *signKey, const char *storepass);

/**
 * \~English
 * Resolve and store DID Document from chain.
 *
 * @param
 *      store                    [in] The handle of DIDStore.
 * @param
 *      did                      [in] The handle to DID.
 * @param
 *      force                    [in] force = true, DIDStore can load document.
                                 force = false, DIDStore can not load document.
 * @return
 *      If no error occurs, return the handle to DID Document.
 *      Otherwise, return NULL.
 */
DID_API DIDDocument *DIDStore_ResolveDID(DIDStore *store, DID *did, bool force);

/******************************************************************************
 * Mnemonic
 *****************************************************************************/
/**
 * \~English
 * Gernerate a random mnemonic.
 *
 * @param
 *      language               [in] The language for DID.
 *                             0: English; 1: French; 2: Spanish;
 *                             3: Chinese_simplified;
 *                             4: Chinese_traditional;
 *                             5: Japanese.
 * @return
 *      mnemonic string.
 */
DID_API const char *Mnemonic_Generate(int language);

/**
 * \~English
 * Free mnemonic buffer.
 *
 * @param
 *      mnemonic               [in] mnemonic buffter.
 */
DID_API void Mnemonic_free(void *mnemonic);

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
DID_API Presentation *Presentation_Create(DID *did, DIDURL *signkey,
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
  * @return
 *      0 on success, -1 if an error occurred.
 */
DID_API int Presentation_Verify(Presentation *pre);

/**
 * \~English
 * Get json context from Presentation.
 *
 * @param
 *      pre                  [in] A handle to Presentation.
 * @param
 *      compact              [in] Json context is compact or not.
 *                           1 represents compact, 0 represents not compact.
 * @param
 *      forsign              [in] Json context needs to sign or not.
 *                           1 represents forsign, 0 represents not forsign
 * @return
 *      If no error occurs, return json context. Return value must be free after
 *      finishing use.
 *      Otherwise, return NULL.
 */
DID_API const char* Presentation_ToJson(Presentation *pre, int compact, int forsign);

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
 *      cred                      [in] The Presentation handle.
 * @return
 *      flase if not valid, true if valid.
 */
DID_API bool Presentation_IsValid(Presentation *pre);

#ifdef __cplusplus
}
#endif

#endif /* __ELA_DID_H__ */