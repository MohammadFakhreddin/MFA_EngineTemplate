
set(LIBRARY_SOURCES)

list(
    APPEND LIBRARY_SOURCES

    "${CMAKE_CURRENT_SOURCE_DIR}/Collision.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Collision.cpp"
)

set(LIBRARY_NAME "Physics")
add_library(${LIBRARY_NAME} ${LIBRARY_SOURCES})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/")