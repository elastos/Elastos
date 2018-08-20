add_definitions(-fPIC)
add_definitions(-fvisibility=hidden)
add_definitions(-DCARRIER_BUILD)

set(ELACARRIER_DEPENDS
     libcrystal
     toxcore0
     libsodium
     flatcc)

set(ELASESSION_DEPENDS
     libcrystal
     pjsip)

add_subdirectory(carrier)
add_subdirectory(session)

if(ENABLE_STATIC)
    add_dependencies(elacarrier-static ${ELACARRIER_DEPENDS})
    add_dependencies(elasession-static ${ELASESSION_DEPENDS} elacarrier-static)
endif()

if(ENABLE_SHARED)
    add_dependencies(elacarrier-shared ${ELACARRIER_DEPENDS})
    add_dependencies(elasession-shared ${ELASESSION_DEPENDS} elacarrier-shared)
endif()
