project(docs)

include(CarrierDefaults)

set(DOXYGEN "doxygen")
set(SPHINXBUILD "sphinx-build")
set(DOXYGEN_CONF "carrier.cfg")

if(APPLE)
    message(FATAL_ERROR
        "Can not build APIs docs on MacOSX")
endif()

find_program(DOXYGEN_PROG NAMES ${DOXYGEN})
if(${DOXYGEN_PROG} STREQUAL "DOXYGEN_PROG-NOTFOUND")
    message(FATAL_ERROR
        "Doxygen is needed to build the docs")
endif()

find_program(SPHINXBUILD_PROG NAMES ${SPHINXBUILD})
if(${SPHINXBUILD_PROG} STREQUAL "SPHINXBUILD_PROG-NOTFOUND")
    message(FATAL_ERROR
        "sphinx-build is needed to build the docs")
endif()

set(CARRIERAPI_INT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(CARRIERAPI_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../src")
set(CARRIERAPI_DOC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source")

set(CARRIERAPI_SRC_HEADERS
    "${CARRIERAPI_SRC_DIR}/carrier/ela_carrier.h"
    "${CARRIERAPI_SRC_DIR}/session/ela_session.h")

file(GLOB_RECURSE CARRIERAPI_DOC_SOURCES *)

string(REPLACE ";" " " CARRIERAPI_SRC_INPUT "${CARRIERAPI_SRC_HEADERS}")

configure_file(
    source/carrier.cfg.in
    carrier.cfg
    "@ONLY")

configure_file(
    source/conf.py.in
    conf.py
    "@ONLY")

add_custom_command(
    OUTPUT html
    DEPENDS ${CARRIERAPI_SRC_HEADERS} ${CARRIERAPI_DOC_SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${DOXYGEN_PROG} ${DOXYGEN_CONF}
    COMMAND ${SPHINXBUILD_PROG}
        -c .
        -b html
        ${CMAKE_CURRENT_SOURCE_DIR}/source
        html)

add_custom_target(docs
    ALL
    COMMENT "Build html APIs documentation"
    DEPENDS html)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
    DESTINATION share/elastos/carrier)
