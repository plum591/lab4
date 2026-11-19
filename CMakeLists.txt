cmake_minimum_required(VERSION 3.20)
project(lab4 VERSION 1.0.0 LANGUAGES CXX)

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

enable_testing()
add_test(NAME allocator_tests COMMAND tests)

install(TARGETS lab4 RUNTIME DESTINATION bin)

if(WIN32)
    set(CPACK_GENERATOR ZIP)
else()
    set(CPACK_GENERATOR DEB)
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "lab4")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
endif()

include(CPack)
