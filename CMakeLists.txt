cmake_minimum_required(VERSION 3.29)
project(com.arisen.core.native LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================================
# Arisen Embedded Build Utils - (Formerly in Engine/Arisen/cmake/Utils.cmake)
# ============================================================================

# ✅ Collect source and header files recursively
function(collect_sources return_var root_dir)
    file(GLOB_RECURSE ALL_CPP_FILES CONFIGURE_DEPENDS ${root_dir}/*.cpp ${root_dir}/*.c)
    file(GLOB_RECURSE ALL_HEADER_FILES CONFIGURE_DEPENDS ${root_dir}/*.h ${root_dir}/*.hpp)
    
    set(SOURCE_FILES "")
    foreach(file_path IN LISTS ALL_CPP_FILES ALL_HEADER_FILES)
        if(NOT file_path MATCHES "/Tests/")
            list(APPEND SOURCE_FILES ${file_path})
        endif()
    endforeach()
    set(${return_var} ${SOURCE_FILES} PARENT_SCOPE)
endfunction()

# ✅ Integrated DXC Setup for autonomous packages
function(setup_dxc_for_target target dxc_root)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(DXC_ARCH "x64")
    else()
        set(DXC_ARCH "x86")
    endif()

    set(DXC_INC "${dxc_root}/inc")
    set(DXC_LIB "${dxc_root}/lib/${DXC_ARCH}/dxcompiler.lib")
    set(DXC_DLL "${dxc_root}/bin/${DXC_ARCH}/dxcompiler.dll")

    if(NOT TARGET dxcompiler)
        add_library(dxcompiler SHARED IMPORTED GLOBAL)
        set_target_properties(dxcompiler PROPERTIES
            IMPORTED_LOCATION "${DXC_DLL}"
            IMPORTED_IMPLIB "${DXC_LIB}"
        )
    endif()

    target_include_directories(${target} PRIVATE "${DXC_INC}")
    target_link_libraries(${target} PUBLIC dxcompiler)
endfunction()

# ============================================================================
# Autonomous Dependencies
# ============================================================================

# Add local spdlog
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/spdlog/CMakeLists.txt")
    add_subdirectory(3rdparty/spdlog)
    set_target_properties(spdlog PROPERTIES FOLDER "3rdparty")
else()
    message(FATAL_ERROR "spdlog not found in 3rdparty! Package is incomplete.")
endif()

# Add local Tracy only when the selected profile owns profiler instrumentation.
if(ARISEN_PROFILER_ENABLED)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/tracy/CMakeLists.txt")
        message(FATAL_ERROR "Tracy source is required by this profiler-enabled build.")
    endif()

    set(TRACY_ENABLE ON CACHE BOOL "Enable Tracy" FORCE)
    set(TRACY_STATIC OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
    add_subdirectory(3rdparty/tracy)
    set_target_properties(TracyClient PROPERTIES FOLDER "3rdparty")
endif()

# ============================================================================
# Subsystems
# ============================================================================

add_subdirectory(Source/Core.Foundation)
add_subdirectory(Source/Core.Diagnostic)
add_subdirectory(Source/Core.HAL)
add_subdirectory(Source/Core.RHI)
add_subdirectory(Source/Core.ShaderCompiler)
