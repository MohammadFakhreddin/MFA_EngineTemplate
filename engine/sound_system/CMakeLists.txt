
set(LIBRARY_SOURCES)

list(
    APPEND LIBRARY_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/SoundSystem.hpp"  
    "${CMAKE_CURRENT_SOURCE_DIR}/SoundSystem.cpp"  
)

set(LIBRARY_NAME "SoundSystem")
add_library(${LIBRARY_NAME} ${LIBRARY_SOURCES})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/")
