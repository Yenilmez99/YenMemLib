cmake_minimum_required(VERSION 3.15)

project("YenMemLib" VERSION 1.0.5 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_FILES
)

add_library(${PROJECT_NAME} STATIC)

if(WIN32)
    list(APPEND SOURCE_FILES
        src/platform_windows.cpp
    )
elseif(UNIX AND NOT APPLE)
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

add_library(YenLib::Memory ALIAS ${PROJECT_NAME})
