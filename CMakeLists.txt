cmake_minimum_required(VERSION 3.14)
project(renamer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    add_compile_options(/W4 /WX-)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O2)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

set(SOURCES
    src/main.cpp
    src/string_utils.cpp
    src/file_utils.cpp
    src/rename_processor.cpp
    src/arg_parser.cpp
)

set(HEADERS
    include/renamer.h
)

add_executable(renamer ${SOURCES} ${HEADERS})

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0" AND CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_link_libraries(renamer PRIVATE stdc++fs)
    endif()
endif()

if(WIN32)
    set_target_properties(renamer PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
else()
    set_target_properties(renamer PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    
    install(TARGETS renamer
        RUNTIME DESTINATION bin
    )
endif()

include(CTest)
if(BUILD_TESTING)
    enable_testing()
    add_test(NAME renamer_help COMMAND renamer --help)
    set_tests_properties(renamer_help PROPERTIES WILL_FAIL FALSE)
endif()
