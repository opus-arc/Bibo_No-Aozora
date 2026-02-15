cmake_minimum_required(VERSION 4.0)
project(Bibo_no-Aozora LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 是否构建可执行程序（用于本地调试）
option(BIBO_BUILD_APP "Build Bibo_no-Aozora executable" ON)

# ----------------------------
# 源文件（自己的代码）
# ----------------------------
set(BIBO_INTERNAL_SOURCES
        main.cpp
        utils/internal/SpectralLoom/tools/Pitch.cpp
        utils/internal/SpectralLoom/tools/RecordPlayer.cpp
        utils/internal/SpectralLoom/tools/SingleTrack.cpp
        utils/internal/SpectralLoom/tools/Chord.cpp
        utils/internal/SpectralLoom/tools/EnvHarmonics.cpp
        utils/internal/SpectralLoom/tools/CelloDynamic.cpp
        utils/internal/SSP_MMC/SSP_MMC.cpp
        utils/internal/SpectralLoom/SpectralLoom.cpp
        utils/internal/SpectralLoom/tools/ComprehensiveExamination.cpp
        backend/database/DatabaseChecker.cpp
        backend/mapper/DataInitializer.cpp
        backend/service/MasterScheduler.cpp
        terminal/terminal.cpp
)


# ----------------------------
# BiboCore_external：纯外部静态库（.a）集合
# - 只负责收集外部依赖
# - 目前强制包含 DuckDB 静态库
# ----------------------------
set(DUCKDB_A "${CMAKE_SOURCE_DIR}/utils/external/duckdb/archives/libduckdb_static.a")
if(NOT EXISTS "${DUCKDB_A}")
    message(FATAL_ERROR "DuckDB static archive not found: ${DUCKDB_A}")
endif()

# 将 DuckDB 静态库导入为 CMake 目标
add_library(duckdb_static STATIC IMPORTED GLOBAL)
set_target_properties(duckdb_static PROPERTIES
        IMPORTED_LOCATION "${DUCKDB_A}"
)

# ----------------------------
# 自动发现第三方库结构：
# utils/external/<name>/include   -> 自动加入头文件搜索路径
# utils/external/<name>/archives  -> 自动收集静态库 .a
# ----------------------------
set(BIBO_EXTERNAL_ROOT "${CMAKE_SOURCE_DIR}/utils/external")

# 自动收集所有 external 子目录
file(GLOB BIBO_EXTERNAL_DIRS LIST_DIRECTORIES true "${BIBO_EXTERNAL_ROOT}/*")

set(BIBO_EXTERNAL_ARCHIVES "")
set(BIBO_EXTERNAL_INCLUDE_DIRS "")

foreach(_dir IN LISTS BIBO_EXTERNAL_DIRS)
    if(IS_DIRECTORY "${_dir}")
        # 1) include 目录（优先）
        if(EXISTS "${_dir}/include" AND IS_DIRECTORY "${_dir}/include")
            list(APPEND BIBO_EXTERNAL_INCLUDE_DIRS "${_dir}/include")
        endif()

        # 2) archives 目录下的所有 .a 作为外部静态库
        if(EXISTS "${_dir}/archives" AND IS_DIRECTORY "${_dir}/archives")
            file(GLOB _archives "${_dir}/archives/*.a")
            list(APPEND BIBO_EXTERNAL_ARCHIVES ${_archives})
        endif()
    endif()
endforeach()

# 去重
list(REMOVE_DUPLICATES BIBO_EXTERNAL_INCLUDE_DIRS)
list(REMOVE_DUPLICATES BIBO_EXTERNAL_ARCHIVES)

# 外部库接口目标（不生成实体文件，只传播依赖）
add_library(BiboCore_external INTERFACE)


# 查找线程库
find_package(Threads REQUIRED)

# macOS: 查找系统 / Homebrew 安装的库
find_library(ACCELERATE_FRAMEWORK Accelerate)

find_library(TAGLIB_LIB tag HINTS /opt/homebrew/lib /usr/local/lib)
find_library(AVCODEC_LIB avcodec HINTS /opt/homebrew/lib /usr/local/lib)
find_library(AVFORMAT_LIB avformat HINTS /opt/homebrew/lib /usr/local/lib)
find_library(AVUTIL_LIB avutil HINTS /opt/homebrew/lib /usr/local/lib)
find_library(SWRESAMPLE_LIB swresample HINTS /opt/homebrew/lib /usr/local/lib)
find_library(SAMPLERATE_LIB samplerate HINTS /opt/homebrew/lib /usr/local/lib)
find_library(YAML_LIB yaml HINTS /opt/homebrew/lib /usr/local/lib)

target_link_libraries(BiboCore_external INTERFACE
        ${ACCELERATE_FRAMEWORK}
        ${TAGLIB_LIB}
        ${AVCODEC_LIB}
        ${AVFORMAT_LIB}
        ${AVUTIL_LIB}
        ${SWRESAMPLE_LIB}
        ${SAMPLERATE_LIB}
        ${YAML_LIB}
)

# 外部库链接关系
# INTERFACE 表示依赖会向使用者传播
target_link_libraries(BiboCore_external INTERFACE
        duckdb_static
        ${BIBO_EXTERNAL_ARCHIVES}
        Threads::Threads
)

# Essentia 若使用 Accelerate(vDSP) 做 FFT，需要链接 Accelerate framework
if(APPLE)
    find_library(ACCELERATE_FRAMEWORK Accelerate)
    if(ACCELERATE_FRAMEWORK)
        target_link_libraries(BiboCore_external INTERFACE "${ACCELERATE_FRAMEWORK}")
    else()
        message(FATAL_ERROR "Accelerate framework not found")
    endif()
endif()

# 将第三方 include 目录传播给所有依赖者
# （注意：CMake 不支持 ** 这种通配 include 目录，这里用真实路径列表替代）
target_include_directories(BiboCore_external INTERFACE ${BIBO_EXTERNAL_INCLUDE_DIRS})

# ----------------------------
# BiboCore_internal：自己的代码编译成静态库
# ----------------------------
add_library(BiboCore_internal STATIC ${BIBO_INTERNAL_SOURCES})

# 项目内部头文件路径
# 如果未来需要给 Swift 暴露 C 接口，可以改为 PUBLIC
target_include_directories(BiboCore_internal PRIVATE
        "${CMAKE_SOURCE_DIR}/utils/external/essentia"
        "${CMAKE_SOURCE_DIR}/utils/external/eigen3"
)

# 内部库依赖外部库
# PUBLIC 表示依赖会向上层传播（例如调试可执行文件）
target_link_libraries(BiboCore_internal PUBLIC BiboCore_external)

# ----------------------------
# 最终产物：合并后的单一静态库 BiboCore.a
# 说明：
# - CMake 不会自动把多个 .a 合并成一个
# - 这里使用 macOS 的 libtool -static 手动合并
# ----------------------------
set(BIBOCORE_MERGED_A "${CMAKE_BINARY_DIR}/BiboCore.a")

# 需要合并的静态库列表
set(BIBOCORE_ARCHIVES_TO_MERGE
        "$<TARGET_FILE:BiboCore_internal>"
        "${DUCKDB_A}"
)

# 将其它外部 .a 也加入合并列表
list(APPEND BIBOCORE_ARCHIVES_TO_MERGE ${BIBO_EXTERNAL_ARCHIVES})

add_custom_command(
        OUTPUT "${BIBOCORE_MERGED_A}"
        COMMAND /usr/bin/libtool -static -o "${BIBOCORE_MERGED_A}" ${BIBOCORE_ARCHIVES_TO_MERGE}
        DEPENDS BiboCore_internal
        COMMENT "正在合并静态库生成 ${BIBOCORE_MERGED_A}"
        VERBATIM
)

# 定义最终目标 BiboCore（默认构建）
add_custom_target(BiboCore ALL DEPENDS "${BIBOCORE_MERGED_A}")

# ----------------------------
# 可选：本地调试用可执行程序
# ----------------------------
if(BIBO_BUILD_APP)
    add_executable(Bibo_no-Aozora ${BIBO_INTERNAL_SOURCES})
    target_link_libraries(Bibo_no-Aozora PRIVATE BiboCore_internal)
endif()

message(STATUS "BiboCore.a 将生成于: ${BIBOCORE_MERGED_A}")