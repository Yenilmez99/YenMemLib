cmake_minimum_required(VERSION 3.15)

if(MINGW)
    set(CMAKE_SYSTEM_NAME Windows)
endif()

project("YenMemLib" VERSION 1.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_FILES
)

add_library(${PROJECT_NAME} STATIC)

if(WIN32)
    list(APPEND SOURCE_FILES
        src/platform_windows.cpp
    )
elseif(UNIX)
    list(APPEND SOURCE_FILES
        src/platform_linux.cpp
    )
endif()

target_sources(${PROJECT_NAME} PRIVATE
${SOURCE_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(MINGW)
    target_link_options(${PROJECT_NAME} PRIVATE
        -static
        -static-libgcc
        -static-libstdc++
    )
endif()

add_library(YenLib::Memory ALIAS ${PROJECT_NAME})
