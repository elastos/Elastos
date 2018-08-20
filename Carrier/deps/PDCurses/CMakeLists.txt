project(PDCurses)

include(CarrierDefaults)
include(ExternalProject)

ExternalProject_Add(
    PDCurses

    PREFIX ${CARRIER_DEPS_BUILD_PREFIX}
    URL "https://github.com/wmcbrine/PDCurses/archive/3.6.tar.gz"
    URL_HASH SHA256=1760cbf3c3f254798e3614efa40206f95fe5bb2ca68eb2be434265d995f078eb
    DOWNLOAD_NAME "PDCurses-3.6.tar.gz"
    DOWNLOAD_DIR ${CARRIER_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    BINARY_DIR "${CARRIER_DEPS_BUILD_PREFIX}/src/PDCurses/wincon"

    PATCH_COMMAND ${PATCH_EXE} -s -p1 < ${CMAKE_CURRENT_LIST_DIR}/PDCurses.patch

    CONFIGURE_COMMAND echo Done
    BUILD_COMMAND nmake -f Makefile.vc WIDE=Y UTF8=Y
    INSTALL_COMMAND copy "..\\curses.h" "${CARRIER_INT_DIST_DIR}\\include" /y
            COMMAND copy "pdcurses.lib" "${CARRIER_INT_DIST_DIR}\\lib" /y
)
