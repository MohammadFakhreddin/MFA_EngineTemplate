########################################

set(EXECUTABLE "TextureMapping")

set(EXECUTABLE_RESOURCES)

list(
    APPEND EXECUTABLE_RESOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/TextureMapping.cpp"
)

add_executable(${EXECUTABLE} ${EXECUTABLE_RESOURCES})


########################################