set(ELACARRIER_DEPENDS
    libcrystal
    toxcore0
    libsodium
    flatcc)

set(ELASESSION_DEPENDS
    libcrystal
    pjsip)

set(ELAFILETRANSFER_DEPENDS
    libcrystal)

add_subdirectory(carrier)
add_subdirectory(session)
add_subdirectory(filetransfer)

if(ENABLE_STATIC)
    add_dependencies(elacarrier-static ${ELACARRIER_DEPENDS})
    add_dependencies(elasession-static ${ELASESSION_DEPENDS} elacarrier-static)
    add_dependencies(elafiletransfer-static ${ELAFILETRANSFER_DEPENDS} elacarrier-static)
    add_dependencies(elafiletransfer-static ${ELAFILETRANSFER_DEPENDS} elasession-static)
endif()

if(ENABLE_SHARED)
    add_dependencies(elacarrier-shared ${ELACARRIER_DEPENDS})
    add_dependencies(elasession-shared ${ELASESSION_DEPENDS} elacarrier-shared)
    add_dependencies(elafiletransfer-shared ${ELAFILETRANSFER_DEPENDS} elacarrier-shared)
    add_dependencies(elafiletransfer-shared ${ELAFILETRANSFER_DEPENDS} elasession-shared)
endif()
