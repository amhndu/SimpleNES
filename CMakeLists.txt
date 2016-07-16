cmake_minimum_required(VERSION 3.1)
project(SimpleNES)

if(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -g")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
endif()

# Add directory containing FindSFML.cmake to module path
#set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/;${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}")

# Add sources
file(GLOB SOURCES
    "${PROJECT_SOURCE_DIR}/main.cpp"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

# Specify include Directory
include_directories("${PROJECT_SOURCE_DIR}/include")

# Find SFML
if (SFML_OS_WINDOWS AND SFML_COMPILER_MSVC)
    find_package( SFML 2 COMPONENTS main audio graphics window system REQUIRED)
else()
    find_package( SFML 2 COMPONENTS audio graphics window system REQUIRED)
endif()
include_directories(${SFML_INCLUDE_DIR})

add_executable(SimpleNES ${SOURCES})
target_link_libraries(SimpleNES ${SFML_LIBRARIES} ${SFML_DEPENDENCIES})

set_property(TARGET SimpleNES PROPERTY CXX_STANDARD 11)
set_property(TARGET SimpleNES PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(SimpleNES)
