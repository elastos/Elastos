Elastos DID core APIs
=========================

Constants
---------

ELA_MAX_DID_LEN
###############

.. doxygendefine:: ELA_MAX_DID_LEN
   :project: DIDAPI

ELA_MAX_DIDURL_LEN
##################

.. doxygendefine:: ELA_MAX_DIDURL_LEN
   :project: DIDAPI

ELA_MAX_ALIAS_LEN
#################

.. doxygendefine:: ELA_MAX_ALIAS_LEN
   :project: DIDAPI

ELA_MAX_TXID_LEN
#################

.. doxygendefine:: ELA_MAX_TXID_LEN
   :project: DIDAPI

ELA_MAX_MNEMONIC_LEN
####################

.. doxygendefine:: ELA_MAX_MNEMONIC_LEN
   :project: DIDAPI

Data types
----------

ELA_DID_FILTER
###############

.. doxygenenum:: ELA_DID_FILTER
   :project: DIDAPI

Property
########

.. doxygenstruct:: Property
   :project: DIDAPI
   :members:

DID
###

.. doxygentypedef:: DID
   :project: DIDAPI

DIDURL
######

.. doxygentypedef:: DIDURL
   :project: DIDAPI

PublicKey
#########

.. doxygentypedef:: PublicKey
   :project: DIDAPI

Credential
##########

.. doxygentypedef:: Credential
   :project: DIDAPI

Presentation
############

.. doxygentypedef:: Presentation
   :project: DIDAPI

Service
#######

.. doxygentypedef:: Service
   :project: DIDAPI

DIDDocument
###########

.. doxygentypedef:: DIDDocument
   :project: DIDAPI

DIDDocumentBuilder
##################

.. doxygentypedef:: DIDDocumentBuilder
   :project: DIDAPI

Issuer
######

.. doxygentypedef:: Issuer
   :project: DIDAPI

DIDStore
########

.. doxygentypedef:: DIDStore
   :project: DIDAPI

DIDAdapter
##########

.. doxygenstruct:: DIDAdapter
   :project: DIDAPI
   :members:

DIDResolver
###########

.. doxygenstruct:: DIDResolver
   :project: DIDAPI
   :members:

JWTBuilder
##########

.. doxygentypedef:: JWTBuilder
   :project: DIDAPI

DIDStore_DIDsCallback
#####################

.. doxygentypedef:: DIDStore_DIDsCallback
   :project: DIDAPI

DIDStore_MergeCallback
######################

.. doxygentypedef:: DIDStore_MergeCallback
   :project: DIDAPI

DIDLogLevel
###########

.. doxygenenum:: DIDLogLevel
   :project: DIDAPI

Functions
---------

DID Log Functions
##################


DID_Log_Init
~~~~~~~~~~~~

.. doxygenfunction:: DID_Log_Init
   :project: DIDAPI


DID Functions
#############

DID_New
~~~~~~~

.. doxygenfunction:: DID_New
   :project: DIDAPI

DID_FromString
~~~~~~~~~~~~~~

.. doxygenfunction:: DID_FromString
   :project: DIDAPI

DID_GetMethod
~~~~~~~~~~~~~

.. doxygenfunction:: DID_GetMethod
   :project: DIDAPI

DID_GetMethodSpecificId
~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DID_GetMethodSpecificId
   :project: DIDAPI

DID_ToString
~~~~~~~~~~~~

.. doxygenfunction:: DID_ToString
   :project: DIDAPI

DID_Compare
~~~~~~~~~~~

.. doxygenfunction:: DID_Compare
   :project: DIDAPI

DID_Resolve
~~~~~~~~~~~~

.. doxygenfunction:: DID_Resolve
   :project: DIDAPI

DID_ResolveAll
~~~~~~~~~~~~~~

.. doxygenfunction:: DID_ResolveAll
   :project: DIDAPI

DID_SetAlias
~~~~~~~~~~~~

.. doxygenfunction:: DID_SetAlias
   :project: DIDAPI

DID_GetAlias
~~~~~~~~~~~~

.. doxygenfunction:: DID_GetAlias
   :project: DIDAPI

DID_GetDeactived
~~~~~~~~~~~~~~~~

.. doxygenfunction:: DID_GetDeactived
   :project: DIDAPI

DID_GetLastTransactionTimestamp
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DID_GetLastTransactionTimestamp
   :project: DIDAPI

DIDURL Functions
################

DIDURL_FromString
~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDURL_FromString
   :project: DIDAPI

DIDURL_NewByDid
~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDURL_NewByDid
   :project: DIDAPI

DIDURL_GetDid
~~~~~~~~~~~~~~

.. doxygenfunction:: DIDURL_GetDid
   :project: DIDAPI

DIDURL_GetFragment
~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDURL_GetFragment
   :project: DIDAPI

DIDURL_ToString
~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDURL_ToString
   :project: DIDAPI

DIDURL_Equals
~~~~~~~~~~~~~

.. doxygenfunction:: DIDURL_Equals
   :project: DIDAPI

DIDURL_Compare
~~~~~~~~~~~~~~

.. doxygenfunction:: DIDURL_Compare
   :project: DIDAPI

DIDURL_Destroy
~~~~~~~~~~~~~~

.. doxygenfunction:: DIDURL_Destroy
   :project: DIDAPI

DIDURL_SetAlias
~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDURL_SetAlias
   :project: DIDAPI

DIDURL_GetAlias
~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDURL_GetAlias
   :project: DIDAPI


DIDDocument Functions
#####################

DIDDocument_FromJson
~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_FromJson
   :project: DIDAPI

DIDDocument_ToJson
~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_ToJson
   :project: DIDAPI

DIDDocument_Destroy
~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_Destroy
   :project: DIDAPI

DIDDocument_IsDeactivated
~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_IsDeactivated
   :project: DIDAPI

DIDDocument_IsGenuine
~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_IsGenuine
   :project: DIDAPI

DIDDocument_IsExpires
~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_IsExpires
   :project: DIDAPI

DIDDocument_IsValid
~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_IsValid
   :project: DIDAPI

DIDDocument_GetSubject
~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_GetSubject
   :project: DIDAPI

DIDDocument_Edit
~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_Edit
   :project: DIDAPI

DIDDocumentBuilder_Destroy
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocumentBuilder_Destroy
   :project: DIDAPI

DIDDocumentBuilder_Seal
~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocumentBuilder_Seal
   :project: DIDAPI

DIDDocumentBuilder_AddPublicKey
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocumentBuilder_AddPublicKey
   :project: DIDAPI

DIDDocumentBuilder_RemovePublicKey
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocumentBuilder_RemovePublicKey
   :project: DIDAPI

DIDDocumentBuilder_AddAuthenticationKey
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocumentBuilder_AddAuthenticationKey
   :project: DIDAPI

DIDDocumentBuilder_RemoveAuthenticationKey
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocumentBuilder_RemoveAuthenticationKey
   :project: DIDAPI

DIDDocumentBuilder_AddAuthorizationKey
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocumentBuilder_AddAuthorizationKey
   :project: DIDAPI

DIDDocumentBuilder_AuthorizationDid
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocumentBuilder_AuthorizationDid
   :project: DIDAPI

DIDDocumentBuilder_AddCredential
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocumentBuilder_AddCredential
   :project: DIDAPI

DIDDocumentBuilder_AddSelfClaimedCredential
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocumentBuilder_AddSelfClaimedCredential
   :project: DIDAPI

DIDDocumentBuilder_RemoveCredential
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocumentBuilder_RemoveCredential
   :project: DIDAPI

DIDDocumentBuilder_AddService
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocumentBuilder_AddService
   :project: DIDAPI

DIDDocumentBuilder_RemoveService
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocumentBuilder_RemoveService
   :project: DIDAPI

DIDDocumentBuilder_SetExpires
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocumentBuilder_SetExpires
   :project: DIDAPI

DIDDocument_GetPublicKeyCount
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_GetPublicKeyCount
   :project: DIDAPI

DIDDocument_GetPublicKeys
~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_GetPublicKeys
   :project: DIDAPI

DIDDocument_GetPublicKey
~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_GetPublicKey
   :project: DIDAPI

DIDDocument_SelectPublicKeys
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_SelectPublicKeys
   :project: DIDAPI

DIDDocument_GetDefaultPublicKey
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_GetDefaultPublicKey
   :project: DIDAPI

DIDDocument_GetAuthenticationCount
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_GetAuthenticationCount
   :project: DIDAPI

DIDDocument_GetAuthenticationKeys
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_GetAuthenticationKeys
   :project: DIDAPI

DIDDocument_GetAuthenticationKey
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_GetAuthenticationKey
   :project: DIDAPI

DIDDocument_SelectAuthenticationKeys
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_SelectAuthenticationKeys
   :project: DIDAPI

DIDDocument_IsAuthenticationKey
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_IsAuthenticationKey
   :project: DIDAPI

DIDDocument_IsAuthorizationKey
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_IsAuthorizationKey
   :project: DIDAPI

DIDDocument_GetAuthorizationCount
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_GetAuthorizationCount
   :project: DIDAPI

DIDDocument_GetAuthorizationKeys
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_GetAuthorizationKeys
   :project: DIDAPI

DIDDocument_GetAuthorizationKey
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_GetAuthorizationKey
   :project: DIDAPI

DIDDocument_SelectAuthorizationKeys
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_SelectAuthorizationKeys
   :project: DIDAPI

DIDDocument_GetCredentialCount
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_GetCredentialCount
   :project: DIDAPI

DIDDocument_GetCredentials
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_GetCredentials
   :project: DIDAPI

DIDDocument_GetCredential
~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_GetCredential
   :project: DIDAPI

DIDDocument_GetServices
~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_GetServices
   :project: DIDAPI

DIDDocument_SelectServices
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_SelectServices
   :project: DIDAPI

DIDDocument_Sign
~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_Sign
   :project: DIDAPI

DIDDocument_SignDigest
~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_SignDigest
   :project: DIDAPI

DIDDocument_Verify
~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_Verify
   :project: DIDAPI

DIDDocument_VerifyDigest
~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_VerifyDigest
   :project: DIDAPI

DIDDocument_SetAlias
~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_SetAlias
   :project: DIDAPI

DIDDocument_GetAlias
~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_GetAlias
   :project: DIDAPI

DIDDocument_GetLastTransactionTimestamp
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_GetLastTransactionTimestamp
   :project: DIDAPI

DIDDocument_GetProofCreater
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_GetProofCreater
   :project: DIDAPI

DIDDocument_GetProofCreatedTime
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_GetProofCreatedTime
   :project: DIDAPI

DIDDocument_GetProofSignature
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_GetProofSignature
   :project: DIDAPI

DIDDocument_GetJwtBuilder
~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDDocument_GetJwtBuilder
   :project: DIDAPI

PublicKey_GetId
~~~~~~~~~~~~~~~

.. doxygenfunction:: PublicKey_GetId
   :project: DIDAPI

PublicKey_GetController
~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: PublicKey_GetController
   :project: DIDAPI

PublicKey_GetPublicKeyBase58
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: PublicKey_GetPublicKeyBase58
   :project: DIDAPI

PublicKey_GetType
~~~~~~~~~~~~~~~~~

.. doxygenfunction:: PublicKey_GetType
   :project: DIDAPI

PublicKey_IsAuthenticationKey
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: PublicKey_IsAuthenticationKey
   :project: DIDAPI

PublicKey_IsAuthorizationKey
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: PublicKey_IsAuthorizationKey
   :project: DIDAPI

Service_GetEndpoint
~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Service_GetEndpoint
   :project: DIDAPI

Service_GetType
~~~~~~~~~~~~~~~~

.. doxygenfunction:: Service_GetType
   :project: DIDAPI

Credential Functions
####################

Credential_ToJson
~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Credential_ToJson
   :project: DIDAPI

Credential_FromJson
~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Credential_FromJson
   :project: DIDAPI

Credential_Destroy
~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Credential_Destroy
   :project: DIDAPI

Credential_GetId
~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Credential_GetId
   :project: DIDAPI

Credential_GetOwner
~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Credential_GetOwner
   :project: DIDAPI

Credential_GetTypeCount
~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Credential_GetTypeCount
   :project: DIDAPI

Credential_GetTypes
~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Credential_GetTypes
   :project: DIDAPI

Credential_GetIssuer
~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Credential_GetIssuer
   :project: DIDAPI

Credential_GetIssuanceDate
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Credential_GetIssuanceDate
   :project: DIDAPI

Credential_GetExpirationDate
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Credential_GetExpirationDate
   :project: DIDAPI

Credential_GetPropertyCount
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Credential_GetPropertyCount
   :project: DIDAPI

Credential_GetProperties
~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Credential_GetProperties
   :project: DIDAPI

Credential_GetProperty
~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Credential_GetProperty
   :project: DIDAPI

Credential_GetProofType
~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Credential_GetProofType
   :project: DIDAPI

Credential_IsExpired
~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Credential_IsExpired
   :project: DIDAPI

Credential_IsGenuine
~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Credential_IsGenuine
   :project: DIDAPI

Credential_IsValid
~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Credential_IsValid
   :project: DIDAPI

Credential_SetAlias
~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Credential_SetAlias
   :project: DIDAPI

Credential_GetAlias
~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Credential_GetAlias
   :project: DIDAPI

Issuer Functions
################

Issuer_Create
~~~~~~~~~~~~~

.. doxygenfunction:: Issuer_Create
   :project: DIDAPI

Issuer_Destroy
~~~~~~~~~~~~~~

.. doxygenfunction:: Issuer_Destroy
   :project: DIDAPI

Issuer_CreateCredential
~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Issuer_CreateCredential
   :project: DIDAPI

Issuer_CreateCredentialByString
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Issuer_CreateCredentialByString
   :project: DIDAPI

Issuer_GetSigner
~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Issuer_GetSigner
   :project: DIDAPI

Issuer_GetSignKey
~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Issuer_GetSignKey
   :project: DIDAPI

DIDStore Functions
##################

DIDStore_Open
~~~~~~~~~~~~~~

.. doxygenfunction:: DIDStore_Open
   :project: DIDAPI

DIDStore_Close
~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDStore_Close
   :project: DIDAPI

DIDStore_ContainsPrivateIdentity
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDStore_ContainsPrivateIdentity
   :project: DIDAPI

DIDStore_InitPrivateIdentity
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDStore_InitPrivateIdentity
   :project: DIDAPI

DIDStore_Synchronize
~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDStore_Synchronize
   :project: DIDAPI

DIDStore_NewDID
~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDStore_NewDID
   :project: DIDAPI

DIDStore_NewDIDByIndex
~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDStore_NewDIDByIndex
   :project: DIDAPI

DIDStore_GetDIDByIndex
~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDStore_GetDIDByIndex
   :project: DIDAPI

DIDStore_ExportMnemonic
~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDStore_ExportMnemonic
   :project: DIDAPI

DIDStore_StoreDID
~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDStore_StoreDID
   :project: DIDAPI

DIDStore_LoadDID
~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDStore_LoadDID
   :project: DIDAPI

DIDStore_ContainsDID
~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDStore_ContainsDID
   :project: DIDAPI

DIDStore_ListDIDs
~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDStore_ListDIDs
   :project: DIDAPI

DIDStore_StoreCredential
~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDStore_StoreCredential
   :project: DIDAPI

DIDStore_LoadCredential
~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDStore_LoadCredential
   :project: DIDAPI

DIDStore_ContainsCredentials
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDStore_ContainsCredentials
   :project: DIDAPI

DIDStore_DeleteCredential
~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDStore_DeleteCredential
   :project: DIDAPI

DIDStore_ListCredentials
~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDStore_ListCredentials
   :project: DIDAPI

DIDStore_SelectCredentials
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDStore_SelectCredentials
   :project: DIDAPI

DIDSotre_ContainsPrivateKeys
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDSotre_ContainsPrivateKeys
   :project: DIDAPI

DIDStore_ContainsPrivateKey
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDStore_ContainsPrivateKey
   :project: DIDAPI

DIDStore_StorePrivateKey
~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDStore_StorePrivateKey
   :project: DIDAPI

DIDStore_DeletePrivateKey
~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDStore_DeletePrivateKey
   :project: DIDAPI

DIDStore_DeactivateDID
~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDStore_DeactivateDID
   :project: DIDAPI


Mnemonic Functions
##################

Mnemonic_Generate
~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Mnemonic_Generate
   :project: DIDAPI

Mnemonic_Free
~~~~~~~~~~~~~

.. doxygenfunction:: Mnemonic_Free
   :project: DIDAPI

Mnemonic_IsValid
~~~~~~~~~~~~~~~~

.. doxygenfunction:: Mnemonic_IsValid
   :project: DIDAPI


Presentation Functions
######################

Presentation_Create
~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Presentation_Create
   :project: DIDAPI

Presentation_Destroy
~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Presentation_Destroy
   :project: DIDAPI

Presentation_ToJson
~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Presentation_ToJson
   :project: DIDAPI

Presentation_FromJson
~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Presentation_FromJson
   :project: DIDAPI

Presentation_GetSigner
~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Presentation_GetSigner
   :project: DIDAPI

Presentation_GetCredentialCount
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Presentation_GetCredentialCount
   :project: DIDAPI

Presentation_GetCredentials
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Presentation_GetCredentials
   :project: DIDAPI

Presentation_GetCredential
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Presentation_GetCredential
   :project: DIDAPI

Presentation_GetType
~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Presentation_GetType
   :project: DIDAPI

Presentation_GetCreatedTime
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Presentation_GetCreatedTime
   :project: DIDAPI

Presentation_GetVerificationMethod
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Presentation_GetVerificationMethod
   :project: DIDAPI

Presentation_GetNonce
~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Presentation_GetNonce
   :project: DIDAPI

Presentation_IsGenuine
~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Presentation_IsGenuine
   :project: DIDAPI

Presentation_IsValid
~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: Presentation_IsValid
   :project: DIDAPI

DIDBackend Functions
####################

DIDBackend_InitializeDefault
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDBackend_InitializeDefault
   :project: DIDAPI

DIDBackend_Initialize
~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDBackend_Initialize
   :project: DIDAPI

DIDBackend_SetTTL
~~~~~~~~~~~~~~~~~

.. doxygenfunction:: DIDBackend_SetTTL
   :project: DIDAPI