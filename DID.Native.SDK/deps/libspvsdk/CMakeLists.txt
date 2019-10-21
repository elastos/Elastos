project(libspvsdk)

include(ProjectDefaults)
include(ExternalProject)
include(ExternalCMakeArgs)

ExternalProject_Add(
    libspvsdk

    PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
    GIT_REPOSITORY "https://github.com/chenyukaola/Elastos.ELA.SPV.Cpp.git"
    GIT_TAG origin/dev
    GIT_PROGRESS TRUE
    DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_INT_DIST_DIR}
        -DARGUMENT_LOG_ENABLE=ON
        -DSPDSDK_DEBUG_ON=OFF
        -DSPV_BUILD_TEST_CASES=OFF
        -DSPV_BUILD_SAMPLE=OFF
        -DSPV_EXTRA_WARNINGS=OFF
        -DBUILD_SHARED_LIBS=OFF
        ${CMAKE_ARGS_INIT}
)


