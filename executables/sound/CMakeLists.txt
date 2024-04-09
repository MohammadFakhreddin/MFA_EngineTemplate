########################################

set(EXECUTABLE "SoundDemo")

set(EXECUTABLE_RESOURCES)

list(
    APPEND EXECUTABLE_RESOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/SoundMain.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/SoundApp.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/SoundApp.hpp"
)

add_executable(${EXECUTABLE} ${EXECUTABLE_RESOURCES})


########################################