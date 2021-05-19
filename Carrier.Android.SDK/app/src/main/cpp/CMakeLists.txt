cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)

set(carrier_include_DIR ${CMAKE_SOURCE_DIR}/../../../native-dist/include)
set(carrier_library_DIR ${CMAKE_SOURCE_DIR}/../../../native-dist/lib)

include_directories(${carrier_include_DIR})
link_directories(${carrier_library_DIR}/${ANDROID_ABI})

set(SOURCES
    init.c
    utils.c
    utilsExt.c
    group.c
    groupUtils.c
    carrier.c
    carrierExtension.c
    carrierHandler.c
    carrierExtUtils.c
    carrierUtils.c
    session.c
    sessionManager.c
    sessionUtils.c
    stream.c
    fileTransfer.c
    fileTransferManager.c
    fileTransferUtils.c)

add_library(carrierjni SHARED ${SOURCES})

target_link_libraries(carrierjni
                      log
                      crystal
                      elacarrier
                      elasession
                      elafiletrans)
