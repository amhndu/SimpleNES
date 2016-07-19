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

# Will add __FILENAME__ macros for all source files, which is the filename without full find_path
# Courtesy of SO
function(define_file_basename_for_sources targetname)
    get_target_property(source_files "${targetname}" SOURCES)
    foreach(sourcefile ${source_files})
        # Get source file's current list of compile definitions.
        get_property(defs SOURCE "${sourcefile}"
            PROPERTY COMPILE_DEFINITIONS)
        # Add the FILE_BASENAME=filename compile definition to the list.
        get_filename_component(basename "${sourcefile}" NAME)
        list(APPEND defs "__FILENAME__=\"${basename}\"")
        # Set the updated compile definitions on the source file.
        set_property(
            SOURCE "${sourcefile}"
            PROPERTY COMPILE_DEFINITIONS ${defs})
    endforeach()
endfunction()

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
define_file_basename_for_sources(SimpleNES)
