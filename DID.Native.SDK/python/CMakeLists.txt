project(eladid_cffi VERSION 1.0.0)

include(ProjectDefaults)

add_custom_command(
    COMMAND python3 -m venv ${CMAKE_CURRENT_BINARY_DIR}/venv
    COMMAND source ${CMAKE_CURRENT_BINARY_DIR}/venv/bin/activate && pip install cffi
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/venv
    VERBATIM
)

add_custom_command(
    COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/include
    COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/../src/ela_did.h ${CMAKE_CURRENT_BINARY_DIR}/include
    COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/../src/jwt/ela_jwt.h ${CMAKE_CURRENT_BINARY_DIR}/include
    COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/ela_did.ffi.h ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/ela_jwt.ffi.h ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../src/ela_did.h ${CMAKE_CURRENT_SOURCE_DIR}/../src/jwt/ela_jwt.h ${CMAKE_CURRENT_SOURCE_DIR}/ela_did.ffi.h ${CMAKE_CURRENT_SOURCE_DIR}/ela_jwt.ffi.h
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/include
    VERBATIM
)

add_custom_command(
    COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/lib
    COMMAND cp -f ${PROJECT_INT_DIST_DIR}/lib/libcjson.a ${CMAKE_CURRENT_BINARY_DIR}/lib
    COMMAND cp -f ${PROJECT_INT_DIST_DIR}/lib/libcurl.a ${CMAKE_CURRENT_BINARY_DIR}/lib
    COMMAND cp -f ${PROJECT_INT_DIST_DIR}/lib/libssl.a ${CMAKE_CURRENT_BINARY_DIR}/lib
    COMMAND cp -f ${PROJECT_INT_DIST_DIR}/lib/libcrypto.a ${CMAKE_CURRENT_BINARY_DIR}/lib
    COMMAND cp -f ${PROJECT_INT_DIST_DIR}/lib/libjansson.a ${CMAKE_CURRENT_BINARY_DIR}/lib
    COMMAND cp -f ${PROJECT_INT_DIST_DIR}/lib/libcjose.a ${CMAKE_CURRENT_BINARY_DIR}/lib
    COMMAND cp -f ${PROJECT_INT_DIST_DIR}/lib/libzip.a ${CMAKE_CURRENT_BINARY_DIR}/lib
    COMMAND cp -f ${PROJECT_INT_DIST_DIR}/lib/libz.a ${CMAKE_CURRENT_BINARY_DIR}/lib
    COMMAND cp -f ${CMAKE_CURRENT_BINARY_DIR}/../src/libeladid.a ${CMAKE_CURRENT_BINARY_DIR}/lib
    COMMAND cp -f ${CMAKE_CURRENT_BINARY_DIR}/../hdkey/libhdkey.a ${CMAKE_CURRENT_BINARY_DIR}/lib
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../src/libeladid.a ${CMAKE_CURRENT_BINARY_DIR}/../hdkey/libhdkey.a
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lib
    VERBATIM
)

add_custom_command(
    COMMAND source ${CMAKE_CURRENT_BINARY_DIR}/venv/bin/activate && python ${CMAKE_CURRENT_SOURCE_DIR}/build.py
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/venv ${CMAKE_CURRENT_BINARY_DIR}/include ${CMAKE_CURRENT_BINARY_DIR}/lib ${CMAKE_CURRENT_SOURCE_DIR}/build.py
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/eladid.cpython-37m-darwin.so
    VERBATIM
)

add_custom_target(eladid_cffi_shared
    ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/eladid.cpython-37m-darwin.so
    VERBATIM
)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/eladid.cpython-37m-darwin.so
    DESTINATION "lib")
