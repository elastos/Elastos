if(__add_submodule_included)
    return()
endif()
set(__add_submodule_included TRUE)

# add_submodule(target
#               DIRECTORY dirname
#               DEPENDS target1 target2 ...)

function(add_submodule name)
    set(_SM_TARGET ${name})
    set(_SM_DIRECTORY ${name})
    set(_SM_DEPENDS)

    set(keyword)

    foreach(arg ${ARGN})
        set(is_value 1)

        if(${arg} STREQUAL "DIRECTORY" OR ${arg} STREQUAL "DEPENDS")
            set(is_value 0)
        endif()

        if(is_value)
            if(keyword)
                if(${keyword} STREQUAL "DIRECTORY")
                    set(_SM_DIRECTORY ${arg})
                elseif(${keyword} STREQUAL "DEPENDS")
                    list(APPEND _SM_DEPENDS ${arg})
                endif()
            else()
                message(AUTHOR_WARNING
                    "value '${arg}' with no previous keyword in add_submodule()")
            endif()
        else()
            set(keyword "${arg}")
        endif()
    endforeach()

    add_subdirectory(${_SM_DIRECTORY})
    if(_SM_DEPENDS)
        add_dependencies(${_SM_TARGET} ${_SM_DEPENDS})
    endif()
endfunction()
