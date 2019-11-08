add_subdirectory(wallet)
add_subdirectory(spvadaptor)

if (ENABLE_SPVADAPTOR_JNI)
    add_subdirectory(jni)
endif()
