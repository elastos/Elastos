set(BUILD_SHARED_LIBS OFF "build static thirdparty")

include(openssl.cmake)
include(boost.cmake)
include(fruit.cmake)
include(sqlite.cmake)
include(catch2.cmake)
include(json.cmake)
include(spdlog.cmake)
