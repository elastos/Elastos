Elastos Carrier session APIs
============================

Data types
----------

ElaStreamType
#############

.. doxygenenum:: ElaStreamType
   :project: CarrierAPI

ElaStreamState
##############

.. doxygenenum:: ElaStreamState
   :project: CarrierAPI

ElaCandidateType
################

.. doxygenenum:: ElaCandidateType
   :project: CarrierAPI

ElaNetworkTopology
##################

.. doxygenenum:: ElaNetworkTopology
   :project: CarrierAPI

ElaAddressInfo
##############

.. doxygenstruct:: ElaAddressInfo
   :project: CarrierAPI
   :members:

ElaTransportInfo
################

.. doxygenstruct:: ElaTransportInfo
   :project: CarrierAPI
   :members:

PortForwardingProtocol
######################

.. doxygenenum:: PortForwardingProtocol
   :project: CarrierAPI

CloseReason
###########

.. doxygenenum:: CloseReason
   :project: CarrierAPI

ElaStreamCallbacks
##################

.. doxygenstruct:: ElaStreamCallbacks
   :project: CarrierAPI
   :members:

ElaSessionRequestCallback
#########################

.. doxygentypedef:: ElaSessionRequestCallback
   :project: CarrierAPI

ElaSessionRequestCompleteCallback
#################################

.. doxygentypedef:: ElaSessionRequestCompleteCallback
   :project: CarrierAPI

Functions
---------

Global session functions
########################

ela_session_init
~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_session_init
   :project: CarrierAPI

ela_session_cleanup
~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_session_cleanup
   :project: CarrierAPI

Session instance functions
##########################

ela_session_new
~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_session_new
   :project: CarrierAPI

ela_session_close
~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_session_close
   :project: CarrierAPI


ela_session_get_peer
~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_session_get_peer
   :project: CarrierAPI

ela_session_set_userdata
~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_session_set_userdata
   :project: CarrierAPI

ela_session_get_userdata
~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_session_get_userdata
   :project: CarrierAPI

ela_session_request
~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_session_request
   :project: CarrierAPI


ela_session_reply_request
~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_session_reply_request
   :project: CarrierAPI

ela_session_start
~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_session_start
   :project: CarrierAPI

Stream functions
################

ela_session_add_stream
~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_session_add_stream
   :project: CarrierAPI

ela_session_remove_stream
~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_session_remove_stream
   :project: CarrierAPI

ela_stream_get_type
~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_stream_get_type
   :project: CarrierAPI

ela_stream_get_state
~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_stream_get_state
   :project: CarrierAPI

ela_stream_get_transport_info
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_stream_get_transport_info
   :project: CarrierAPI

ela_stream_write
~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_stream_write
   :project: CarrierAPI

ela_stream_open_channel
~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_stream_open_channel
   :project: CarrierAPI

ela_stream_close_channel
~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_stream_close_channel
   :project: CarrierAPI

ela_stream_write_channel
~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_stream_write_channel
   :project: CarrierAPI

ela_stream_pend_channel
~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_stream_pend_channel
   :project: CarrierAPI

ela_stream_resume_channel
~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_stream_resume_channel
   :project: CarrierAPI

PortForwarding functions
########################

ela_session_add_service
~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_session_add_service
   :project: CarrierAPI

ela_session_remove_service
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_session_remove_service
   :project: CarrierAPI

ela_stream_open_port_forwarding
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_stream_open_port_forwarding
   :project: CarrierAPI

ela_stream_close_port_forwarding
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: ela_stream_close_port_forwarding
   :project: CarrierAPI

