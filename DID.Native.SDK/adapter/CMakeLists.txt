add_subdirectory(spvadapter)

if (ENABLE_SPVADAPTER_JNI)
    add_subdirectory(jni)
endif()
