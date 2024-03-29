# -*- cmake -*-

include(00-Common)
include(Prebuilt)

set(LSCRIPT_INCLUDE_DIRS
    ${LIBS_OPEN_DIR}/lscript
    ${LIBS_OPEN_DIR}/lscript/lscript_compile
    ${LIBS_OPEN_DIR}/lscript/lscript_execute    
    ${LIBS_OPEN_DIR}/lscript/lscript_execute_mono
    )

set(LSCRIPT_LIBRARIES
    lscript_compile
    lscript_execute    
    lscript_library
    )

if(LINUX)
    use_prebuilt_binary(tailslide)
endif()

set(LSCRIPT_EXECUTE_MONO_LIBRARIES lscript_execute_mono)
