cmake_minimum_required(VERSION 2.6)
project(SimpleNES)

if(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -g")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
endif()
# How do you specify C++11 for MSVC ?

IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
    add_definitions(-DDEBUG)
ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)

# Add directory containing FindSFML.cmake to module path
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/;${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}")

# Add sources
file(GLOB SOURCES
    "${PROJECT_SOURCE_DIR}/main.cpp"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

# Specify include Directory
include_directories("${PROJECT_SOURCE_DIR}/include")

#file(COPY resources DESTINATION .)

# Find SFML
#if (SFML_OS_WINDOWS AND SFML_COMPILER_MSVC)
#    find_package( SFML 2 COMPONENTS main graphics window system )
#else()
#    find_package( SFML 2 COMPONENTS graphics window system )
#endif()

#if(SFML_FOUND)
#        include_directories(${SFML_INCLUDE_DIR})
#else()
#        set(SFML_ROOT "" CACHE PATH "SFML top-level directory")
#        message("\n-> SFML directory not found. Set SFML_ROOT to SFML's top-level path (containing \"include\" and \"lib\" directories).")
#        message("-> Make sure the SFML libraries with the same configuration (Release/Debug, Static/Dynamic) exist.\n")
#endif()

add_executable(SimpleNES ${SOURCES})

#target_link_libraries(SimpleNES ${SFML_LIBRARIES})
target_link_libraries(SimpleNES)
