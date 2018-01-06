cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)

set(carrier_include_DIR ${CMAKE_SOURCE_DIR}/../../../native-dist/include)
set(carrier_library_DIR ${CMAKE_SOURCE_DIR}/../../../native-dist/libs)

add_library(libcarrier SHARED IMPORTED)
set_target_properties(libcarrier PROPERTIES IMPORTED_LOCATION
                      ${carrier_library_DIR}/${ANDROID_ABI}/libcarrier-native.a)

add_library(carrierjni SHARED
            init.c
            utils.c
            utilsExt.c
            carrier.c
            carrierHandler.c
            carrierUtils.c
            session.c
            sessionManager.c
            sessionUtils.c
            stream.c)

target_include_directories(carrierjni PRIVATE
                           ${carrier_include_DIR})

target_link_libraries(carrierjni
                      log
                      libcarrier)
