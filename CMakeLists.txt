cmake_minimum_required(VERSION VERSION 3.16.0)

project ("MFA_EngineV2" C CXX)

set_property (GLOBAL PROPERTY USE_FOLDERS ON)

set (CMAKE_CXX_STANDARD 20)

set (CMAKE_CXX_STANDARD_REQUIRED ON)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(WINDOWS TRUE)
endif()

if(MSVC)
    add_definitions (-D_CRT_SECURE_NO_WARNINGS)
    add_definitions (/MDd)
else()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
endif()

if(${CMAKE_BUILD_TYPE} MATCHES Release)
    add_definitions(-DNDEBUG)
    message(STATUS "Runnning on release mode")
    message(STATUS ${CMAKE_CXX_FLAGS_RELEASE})
else()
    add_definitions(-D_DEBUG)
    message(STATUS "Running on debug mode")
    message(STATUS ${CMAKE_CXX_FLAGS_DEBUG})
endif()

if(LINUX)
    set(CMAKE_THREAD_LIBS_INIT "-lpthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    set(CMAKE_HAVE_THREADS_LIBRARY 1)
    set(CMAKE_USE_WIN32_THREADS_INIT 0)
    set(CMAKE_USE_PTHREADS_INIT 1)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()

### OpenMP #############################################

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
    link_libraries(OpenMP::OpenMP_CXX)
endif()

### Imgui ###############################################

add_subdirectory("${CMAKE_SOURCE_DIR}/engine/libs/imgui")
include_directories("${CMAKE_SOURCE_DIR}/engine/libs/imgui")
link_libraries(Imgui)

### Vulkan ##############################################

find_package(Vulkan REQUIRED)
message(STATUS "Vulkan include directories are ${Vulkan_INCLUDE_DIRS}")
include_directories(${Vulkan_INCLUDE_DIRS})
link_libraries(Vulkan::Vulkan)

### glm ##################################################

add_definitions(-DGLM_FORCE_SILENT_WARNINGS)
add_definitions(-DGLM_FORCE_DEPTH_ZERO_TO_ONE)
add_subdirectory("${CMAKE_SOURCE_DIR}/engine/libs/glm/glm")
include_directories("${CMAKE_SOURCE_DIR}/engine/libs/glm/glm")
link_libraries(glm)

### Working directory

add_compile_definitions("ASSET_DIR=${CMAKE_SOURCE_DIR}/assets")

### LibConfig ############################################

add_subdirectory("${CMAKE_SOURCE_DIR}/engine/libs/libconfig")
include_directories("${CMAKE_SOURCE_DIR}/engine/libs/libconfig")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/engine/libs/libconfig/stb_image")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/engine/libs/libconfig/nlohmann")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/engine/libs/libconfig/tiny_gltf_loader")
link_libraries(LibConfig)

## SDL #################################################

find_package(SDL2 REQUIRED)
message(STATUS "SDL include directories are ${SDL2_INCLUDE_DIRS}")
include_directories(${SDL2_INCLUDE_DIRS})
message(STATUS "SDL libraries are ${SDL2_LIBRARIES}")
link_libraries(${SDL2_LIBRARIES})

### Bedrock ##############################################

add_subdirectory("${CMAKE_SOURCE_DIR}/engine/bedrock")
include_directories("${CMAKE_SOURCE_DIR}/engine/bedrock")
link_libraries(Bedrock)

### Asset system #########################################

add_subdirectory("${CMAKE_SOURCE_DIR}/engine/asset_system")
include_directories("${CMAKE_SOURCE_DIR}/engine/asset_system")
link_libraries(AssetSystem)

### Importer #############################################

add_subdirectory("${CMAKE_SOURCE_DIR}/engine/importer")
include_directories("${CMAKE_SOURCE_DIR}/engine/importer")
link_libraries(Importer)

### JobSystem ############################################

add_subdirectory("${CMAKE_SOURCE_DIR}/engine/job_system")
include_directories("${CMAKE_SOURCE_DIR}/engine/job_system")
link_libraries(JobSystem)

### Physics #############################################

add_subdirectory("${CMAKE_SOURCE_DIR}/engine/physics")
include_directories("${CMAKE_SOURCE_DIR}/engine/physics")
link_libraries(Physics)

### Renderer #############################################

add_subdirectory("${CMAKE_SOURCE_DIR}/engine/render_system")
include_directories("${CMAKE_SOURCE_DIR}/engine/render_system")
link_libraries(RenderSystem)

### Executables

### MeshViewer ##############################################

add_subdirectory("${CMAKE_SOURCE_DIR}/executables/mesh-viewer")

### TextureMapping ##########################################

add_subdirectory("${CMAKE_SOURCE_DIR}/executables/texture-mapping")

#############################################################