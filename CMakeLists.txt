cmake_minimum_required(VERSION 3.20)
set(LAB4_VERSION_PATCH "0" CACHE STRING "Patch version used for CI releases")
project(lab4 VERSION 1.0.${LAB4_VERSION_PATCH} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

add_executable(lab4 main.cpp PoolAllocator.h SLLContainer.h)

install(TARGETS lab4 RUNTIME DESTINATION bin)

if(WIN32)
    set(CPACK_GENERATOR ZIP)
else()
    set(CPACK_GENERATOR DEB)
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "lab4")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
endif()

set(CPACK_PACKAGE_FILE_NAME "lab4-v${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}")
include(CPack)