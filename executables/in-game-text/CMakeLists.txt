########################################

set(EXECUTABLE "InGameText")

set(EXECUTABLE_RESOURCES)

list(
    APPEND EXECUTABLE_RESOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/InGameTextMain.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/InGameTextApp.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/InGameTextApp.hpp"
)

add_executable(${EXECUTABLE} ${EXECUTABLE_RESOURCES})


########################################