if(__check_build_option_included)
    return()
endif()
set(__check_build_option_included TRUE)

function(check_build_option option)
    if(NOT ${option} AND "$ENV{${option}}" STREQUAL "")
        set(${option} NOTFOUND  PARENT_SCOPE)
    elseif(NOT ("$ENV{${option}}" STREQUAL ""))
        set(${option} "$ENV{${option}}" PARENT_SCOPE)
    elseif(NOT (${option} STREQUAL ""))
        set(ENV{${option}} ${${option}})
    endif()
endfunction()

function(check_build_option_path option)
    if(NOT ${option} AND "$ENV{${option}}" STREQUAL "")
        set(${option} NOTFOUND PARENT_SCOPE)
    elseif(NOT ("$ENV{${option}}" STREQUAL ""))
        set(${option} "$ENV{${option}}" PARENT_SCOPE)
    elseif(NOT (${option} STREQUAL ""))
        get_filename_component(_CBO_${option}
            "${${option}}"
            ABSOLUTE
            BASE_DIR
            "${CMAKE_CURRENT_BINARY_DIR}")

        if(NOT EXISTS ${_CBO_${option}})
            set(${option} NOTFOUND PARENT_SCOPE)
        else()
            set(${option} "${_CBO_${option}}" PARENT_SCOPE)
            set(ENV{${option}} ${_CBO_${option}})
        endif()
    endif()
endfunction()
