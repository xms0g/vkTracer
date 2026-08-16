function (add_slang_shader_target TARGET)
    cmake_parse_arguments ("SHADER" "" "SOURCES" "OUTPUT_NAME" ${ARGN})
    set (SHADERS_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
    set (ENTRY_POINTS -entry vertMain -entry fragMain -entry compMain)

    if (SHADER_OUTPUT_NAME)
        set (OUTPUT_FILE ${SHADERS_DIR}/${SHADER_OUTPUT_NAME})
    else()
        set (OUTPUT_FILE ${SHADERS_DIR}/slang.spv)
    endif()

    add_custom_command (
            OUTPUT ${SHADERS_DIR}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
    )
    add_custom_command (
            OUTPUT  ${OUTPUT_FILE}
            COMMAND ${SLANGC_EXECUTABLE} ${SHADER_SOURCES} -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name ${ENTRY_POINTS} -o ${OUTPUT_FILE}
            WORKING_DIRECTORY ${SHADERS_DIR}
            DEPENDS ${SHADERS_DIR} ${SHADER_SOURCES}
            COMMENT "Compiling Slang Shaders: ${SHADER_OUTPUT_NAME}"
            VERBATIM
    )
    add_custom_target (${TARGET} DEPENDS ${OUTPUT_FILE})
endfunction()