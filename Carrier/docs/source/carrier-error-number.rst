Elastos Carrier error number
============================

Error number format
-------------------

The Carrier error numbering space is extensible. The numbering space has the following internal structure.

+----------+-----------+--------+
|     0    |   1 ~ 7   | 8 ~ 31 |
+==========+===========+========+
| Severity | Facility  |  Code  |
+----------+-----------+--------+

An error number value has 32 bits divided into three fields: a severity code, a facility code, and an error code. The severity code indicates whether the return value represents information, warning, or error. The facility code identifies the area of the system responsible for the error. The error code is a unique number that is assigned to represent the exception.

Format details
##############

* Severity - indicates a error

  - 1 - Failure

* Facility - indicates the Carrier module that is responsible for the error. Available facility codes are shown below:

  - 1 - General
  - 2 - System
  - 3 - HTTP
  - 4 - Reserved/not used
  - 5 - ICE
  - 6 - DHT

* Code - is the facility's error code

Example
#######

0x86000021

* 0x8 - Error
* 0x6 - DHT
* 0x21 - The error number(ELAERR_BAD_BOOTSTRAP_PORT)

Error codes
-----------

ELAERR_INVALID_ARGS
###################

.. doxygendefine:: ELAERR_INVALID_ARGS
   :project: CarrierAPI


ELAERR_OUT_OF_MEMORY
####################

.. doxygendefine:: ELAERR_OUT_OF_MEMORY
   :project: CarrierAPI

ELAERR_BUFFER_TOO_SMALL
#######################

.. doxygendefine:: ELAERR_BUFFER_TOO_SMALL
   :project: CarrierAPI

ELAERR_BAD_PERSISTENT_DATA
##########################

.. doxygendefine:: ELAERR_BAD_PERSISTENT_DATA
   :project: CarrierAPI

ELAERR_INVALID_PERSISTENCE_FILE
###############################

.. doxygendefine:: ELAERR_INVALID_PERSISTENCE_FILE
   :project: CarrierAPI

ELAERR_INVALID_CONTROL_PACKET
#############################

.. doxygendefine:: ELAERR_INVALID_CONTROL_PACKET
   :project: CarrierAPI

ELAERR_INVALID_CREDENTIAL
#########################

.. doxygendefine:: ELAERR_INVALID_CREDENTIAL
   :project: CarrierAPI

ELAERR_ALREADY_RUN
##################

.. doxygendefine:: ELAERR_ALREADY_RUN
   :project: CarrierAPI

ELAERR_NOT_READY
################

.. doxygendefine:: ELAERR_NOT_READY
   :project: CarrierAPI

ELAERR_NOT_EXIST
################

.. doxygendefine:: ELAERR_NOT_EXIST
   :project: CarrierAPI

ELAERR_ALREADY_EXIST
####################

.. doxygendefine:: ELAERR_ALREADY_EXIST
   :project: CarrierAPI

ELAERR_NO_MATCHED_REQUEST
#########################

.. doxygendefine:: ELAERR_NO_MATCHED_REQUEST
   :project: CarrierAPI

ELAERR_INVALID_USERID
#####################

.. doxygendefine:: ELAERR_INVALID_USERID
   :project: CarrierAPI

ELAERR_INVALID_NODEID
#####################

.. doxygendefine:: ELAERR_INVALID_NODEID
   :project: CarrierAPI

ELAERR_WRONG_STATE
##################

.. doxygendefine:: ELAERR_WRONG_STATE
   :project: CarrierAPI

ELAERR_BUSY
###########

.. doxygendefine:: ELAERR_BUSY
   :project: CarrierAPI

ELAERR_LANGUAGE_BINDING
#######################

.. doxygendefine:: ELAERR_LANGUAGE_BINDING
   :project: CarrierAPI

ELAERR_ENCRYPT
##############

.. doxygendefine:: ELAERR_ENCRYPT
   :project: CarrierAPI

ELAERR_SDP_TOO_LONG
###################

.. doxygendefine:: ELAERR_SDP_TOO_LONG
   :project: CarrierAPI

ELAERR_INVALID_SDP
##################

.. doxygendefine:: ELAERR_INVALID_SDP
   :project: CarrierAPI

ELAERR_NOT_IMPLEMENTED
######################

.. doxygendefine:: ELAERR_NOT_IMPLEMENTED
   :project: CarrierAPI

ELAERR_LIMIT_EXCEEDED
#####################

.. doxygendefine:: ELAERR_LIMIT_EXCEEDED
   :project: CarrierAPI

ELAERR_PORT_ALLOC
#################

.. doxygendefine:: ELAERR_PORT_ALLOC
   :project: CarrierAPI

ELAERR_BAD_PROXY_TYPE
#####################

.. doxygendefine:: ELAERR_BAD_PROXY_TYPE
   :project: CarrierAPI

ELAERR_BAD_PROXY_HOST
#####################

.. doxygendefine:: ELAERR_BAD_PROXY_HOST
   :project: CarrierAPI


ELAERR_BAD_PROXY_PORT
#####################

.. doxygendefine:: ELAERR_BAD_PROXY_PORT
   :project: CarrierAPI

ELAERR_PROXY_NOT_AVAILABLE
##########################

.. doxygendefine:: ELAERR_PROXY_NOT_AVAILABLE
   :project: CarrierAPI

ELAERR_ENCRYPTED_PERSISTENT_DATA
################################

.. doxygendefine:: ELAERR_ENCRYPTED_PERSISTENT_DATA
   :project: CarrierAPI

ELAERR_BAD_BOOTSTRAP_HOST
#########################

.. doxygendefine:: ELAERR_BAD_BOOTSTRAP_HOST
   :project: CarrierAPI

ELAERR_BAD_BOOTSTRAP_PORT
#########################

.. doxygendefine:: ELAERR_BAD_BOOTSTRAP_PORT
   :project: CarrierAPI

ELAERR_TOO_LONG
###############

.. doxygendefine:: ELAERR_TOO_LONG
   :project: CarrierAPI


ELAERR_ADD_SELF
###############

.. doxygendefine:: ELAERR_ADD_SELF
   :project: CarrierAPI

ELAERR_BAD_ADDRESS
##################

.. doxygendefine:: ELAERR_BAD_ADDRESS
   :project: CarrierAPI

ELAERR_FRIEND_OFFLINE
#####################

.. doxygendefine:: ELAERR_FRIEND_OFFLINE
   :project: CarrierAPI

ELAERR_UNKNOWN
##############

.. doxygendefine:: ELAERR_UNKNOWN
   :project: CarrierAPI
