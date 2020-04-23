Elastos Carrier core APIs
=========================

Constants
---------

ELA_MAX_ADDRESS_LEN
###################

.. doxygendefine:: ELA_MAX_ADDRESS_LEN
   :project: CarrierAPI

ELA_MAX_ID_LEN
##############

.. doxygendefine:: ELA_MAX_ID_LEN
   :project: CarrierAPI

ELA_MAX_USER_NAME_LEN
#####################

.. doxygendefine:: ELA_MAX_USER_NAME_LEN
   :project: CarrierAPI

ELA_MAX_USER_DESCRIPTION_LEN
############################

.. doxygendefine:: ELA_MAX_USER_DESCRIPTION_LEN
   :project: CarrierAPI

ELA_MAX_PHONE_LEN
#################

.. doxygendefine:: ELA_MAX_PHONE_LEN
   :project: CarrierAPI

ELA_MAX_EMAIL_LEN
#################

.. doxygendefine:: ELA_MAX_EMAIL_LEN
   :project: CarrierAPI

ELA_MAX_REGION_LEN
##################

.. doxygendefine:: ELA_MAX_REGION_LEN
   :project: CarrierAPI

ELA_MAX_GENDER_LEN
##################

.. doxygendefine:: ELA_MAX_GENDER_LEN
   :project: CarrierAPI

ELA_MAX_NODE_NAME_LEN
#####################

.. doxygendefine:: ELA_MAX_NODE_NAME_LEN
   :project: CarrierAPI

ELA_MAX_NODE_DESCRIPTION_LEN
############################

.. doxygendefine:: ELA_MAX_NODE_DESCRIPTION_LEN
   :project: CarrierAPI

ELA_MAX_APP_MESSAGE_LEN
#######################

.. doxygendefine:: ELA_MAX_APP_MESSAGE_LEN
   :project: CarrierAPI

ELA_MAX_APP_BIG_MESSAGE_LEN
###########################

.. doxygendefine:: ELA_MAX_APP_BIG_MESSAGE_LEN
   :project: CarrierAPI

Data types
----------

Bootstrap
#########

.. doxygenstruct:: BootstrapNode
   :project: CarrierAPI
   :members:

ElaOptions
##########

.. doxygenstruct:: ElaOptions
   :project: CarrierAPI
   :members:


ElaConnectionStatus
###################

.. doxygenenum:: ElaConnectionStatus
   :project: CarrierAPI


ElaPresenceStatus
#################

.. doxygenenum:: ElaPresenceStatus
   :project: CarrierAPI

ElaLogLevel
###########

.. doxygenenum:: ElaLogLevel
   :project: CarrierAPI

ElaUserInfo
###########

.. doxygenstruct:: ElaUserInfo
   :project: CarrierAPI
   :members:

ElaFriendInfo
#############

.. doxygenstruct:: ElaFriendInfo
   :project: CarrierAPI
   :members:

ElaCallbacks
############

.. doxygenstruct:: ElaCallbacks
   :project: CarrierAPI
   :members:

ElaFriendsIterateCallback
#########################

.. doxygentypedef:: ElaFriendsIterateCallback
   :project: CarrierAPI

ElaFriendInviteResponseCallback
###############################

.. doxygentypedef:: ElaFriendInviteResponseCallback
   :project: CarrierAPI

Functions
---------

Carrier instance
################

ela_new
~~~~~~~

.. doxygenfunction:: ela_new
   :project: CarrierAPI

ela_run
~~~~~~~

.. doxygenfunction:: ela_run
   :project: CarrierAPI

ela_kill
~~~~~~~~

.. doxygenfunction:: ela_kill
   :project: CarrierAPI

ela_is_ready
~~~~~~~~~~~~

.. doxygenfunction:: ela_is_ready
   :project: CarrierAPI

Node Information
################

ela_get_address
~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_get_address
   :project: CarrierAPI

ela_get_nodeid
~~~~~~~~~~~~~~

.. doxygenfunction:: ela_get_nodeid
   :project: CarrierAPI

ela_get_userid
~~~~~~~~~~~~~~

.. doxygenfunction:: ela_get_userid
   :project: CarrierAPI

ela_get_id_by_address
~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_get_id_by_address
   :project: CarrierAPI

ela_set_self_nospam
~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_set_self_nospam
   :project: CarrierAPI

ela_get_self_nospam
~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_get_self_nospam
   :project: CarrierAPI

ela_set_self_info
~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_set_self_info
   :project: CarrierAPI

ela_get_self_info
~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_get_self_info
   :project: CarrierAPI

ela_set_self_presence
~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_set_self_presence
   :project: CarrierAPI

ela_get_self_presence
~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_get_self_presence
   :project: CarrierAPI


Friend & interaction
####################

ela_get_friends
~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_get_friends
   :project: CarrierAPI

ela_get_friend_info
~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_get_friend_info
   :project: CarrierAPI

ela_set_friend_label
~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_set_friend_label
   :project: CarrierAPI

ela_is_friend
~~~~~~~~~~~~~

.. doxygenfunction:: ela_is_friend
   :project: CarrierAPI

ela_add_friend
~~~~~~~~~~~~~~

.. doxygenfunction:: ela_add_friend
   :project: CarrierAPI

ela_accept_friend
~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_accept_friend
   :project: CarrierAPI

ela_remove_friend
~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_remove_friend
   :project: CarrierAPI

ela_send_friend_message
~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_send_friend_message
   :project: CarrierAPI

ela_invite_friend
~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_invite_friend
   :project: CarrierAPI

ela_reply_friend_invite
~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_reply_friend_invite
   :project: CarrierAPI


Utility functions
#################

ela_get_version
~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_get_version
   :project: CarrierAPI

ela_address_is_valid
~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_address_is_valid
   :project: CarrierAPI

ela_id_is_valid
~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_id_is_valid
   :project: CarrierAPI

ela_get_error
~~~~~~~~~~~~~

.. doxygenfunction:: ela_get_error
   :project: CarrierAPI

ela_clear_error
~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_clear_error
   :project: CarrierAPI
