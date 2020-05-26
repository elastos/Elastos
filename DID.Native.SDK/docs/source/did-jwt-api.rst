Elastos DID Jwt APIs
============================

Data types
----------

JWS
####

.. doxygentypedef:: JWS
   :project: DIDAPI

Functions
---------

JWtBuilder Functions
########################

JWTBuilder_Destroy
~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: JWTBuilder_Destroy
   :project: DIDAPI

JWTBuilder_SetHeader
~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: JWTBuilder_SetHeader
   :project: DIDAPI

JWTBuilder_SetClaim
~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: JWTBuilder_SetClaim
   :project: DIDAPI

JWTBuilder_SetClaimWithJson
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: JWTBuilder_SetClaimWithJson
   :project: DIDAPI

JWTBuilder_SetClaimWithBoolean
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: JWTBuilder_SetClaimWithBoolean
   :project: DIDAPI

JWTBuilder_SetIssuer
~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: JWTBuilder_SetIssuer
   :project: DIDAPI

JWTBuilder_SetSubject
~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: JWTBuilder_SetSubject
   :project: DIDAPI

JWTBuilder_SetAudience
~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: JWTBuilder_SetAudience
   :project: DIDAPI

JWTBuilder_SetNotBefore
~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: JWTBuilder_SetNotBefore
   :project: DIDAPI

JWTBuilder_SetIssuedAt
~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: JWTBuilder_SetIssuedAt
   :project: DIDAPI

JWTBuilder_SetId
~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: JWTBuilder_SetId
   :project: DIDAPI

JWTBuilder_Sign
~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: JWTBuilder_Sign
   :project: DIDAPI

JWTBuilder_Compact
~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: JWTBuilder_Compact
   :project: DIDAPI

JWTBuilder_Reset
~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: JWTBuilder_Reset
   :project: DIDAPI


JWTParser Functions
###################

JWTParser_Parse
~~~~~~~~~~~~~~~~

.. doxygenfunction:: JWTParser_Parse
   :project: DIDAPI

JWS Functions
##############

JWS_Destroy
~~~~~~~~~~~~~~~

.. doxygenfunction:: JWS_Destroy
   :project: DIDAPI

JWS_GetHeader
~~~~~~~~~~~~~~~~~

.. doxygenfunction:: JWS_GetHeader
   :project: DIDAPI


JWS_GetAlgorithm
~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: JWS_GetAlgorithm
   :project: DIDAPI

JWS_GetKeyId
~~~~~~~~~~~~~~~

.. doxygenfunction:: JWS_GetKeyId
   :project: DIDAPI

JWS_GetClaim
~~~~~~~~~~~~~

.. doxygenfunction:: JWS_GetClaim
   :project: DIDAPI

JWS_GetClaimAsJson
~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: JWS_GetClaimAsJson
   :project: DIDAPI

JWS_GetClaimAsInteger
~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: JWS_GetClaimAsInteger
   :project: DIDAPI

JWS_GetClaimAsBoolean
~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: JWS_GetClaimAsBoolean
   :project: DIDAPI

JWS_GetIssuer
~~~~~~~~~~~~~~

.. doxygenfunction:: JWS_GetIssuer
   :project: DIDAPI

JWS_GetAudience
~~~~~~~~~~~~~~~~

.. doxygenfunction:: JWS_GetAudience
   :project: DIDAPI

JWS_GetId
~~~~~~~~~~

.. doxygenfunction:: JWS_GetId
   :project: DIDAPI

JWS_GetExpiration
~~~~~~~~~~~~~~~~~

.. doxygenfunction:: JWS_GetExpiration
   :project: DIDAPI

JWS_GetNotBefore
~~~~~~~~~~~~~~~~~

.. doxygenfunction:: JWS_GetNotBefore
   :project: DIDAPI

JWS_GetIssuedAt
~~~~~~~~~~~~~~~~

.. doxygenfunction:: JWS_GetIssuedAt
   :project: DIDAPI