cmake_minimum_required(VERSION 3.0.0)
project(Project VERSION 0.1.0)

# C++ 11
set(CMAKE_CXX_STANDARD 11)

# Include
set(INCLUDE ${PROJECT_SOURCE_DIR}/include)
include_directories(${INCLUDE})


# Lib
# set(GLFW_LINK /usr/local/lib/libglfw.3.dylib)
# link_libraries(${GLFW_LINK})
if (MSVC)
    set(LIB ${PROJECT_SOURCE_DIR}/lib)
    link_directories(${LIB})
endif()

if(APPLE)
    set(GLFW ${PROJECT_SOURCE_DIR}/lib/libglfw/libglfw.dylib)
    set(ASSIMP ${PROJECT_SOURCE_DIR}/lib/assimp/libassimpd.dylib)
    link_libraries(${GLFW})
    link_libraries(${ASSIMP})
endif()
# link_directories(${GLFW})

# Source
file(GLOB_RECURSE SOURCES 
        src/*.c
        src/main.cpp
)

add_executable(Project ${SOURCES})

if (MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Project)
    target_link_libraries(Project glfw3 opengl32 assimp)
	set_target_properties(Project PROPERTIES LINK_FLAGS "/ignore:4098 /ignore:4099")
    
endif()
# APPLE
if (APPLE)
    set_target_properties(Project PROPERTIES LINK_FLAGS "-Wl, -F/Library/Frameworks")
    target_link_libraries(Project "-framework OpenGL")
endif()

include(CTest)
enable_testing()


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
