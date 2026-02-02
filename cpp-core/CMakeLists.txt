cmake_minimum_required(VERSION 4.0)
project(Bibo_no-Aozora)


set(CMAKE_CXX_STANDARD 20)


add_executable(
        Bibo_no-Aozora main.cpp
        utils/internal/SpectralLoom/tools/Pitch.cpp
        utils/internal/SpectralLoom/tools/Pitch.h
        utils/external/miniaudio.h
        utils/internal/SpectralLoom/tools/RecordPlayer.cpp
        utils/internal/SpectralLoom/tools/RecordPlayer.h
        utils/internal/SpectralLoom/tools/SingleTrack.h
        utils/internal/SpectralLoom/tools/SingleTrack.cpp
        utils/internal/SpectralLoom/tools/Chord.cpp
        utils/internal/SpectralLoom/tools/Chord.h
        utils/internal/SpectralLoom/tools/EnvHarmonics.cpp
        utils/internal/SpectralLoom/tools/EnvHarmonics.h
        utils/internal/SpectralLoom/tools/CelloDynamic.cpp
        utils/internal/SpectralLoom/tools/CelloDynamic.h
        utils/internal/SSP_MMC/SSP_MMC.cpp
        utils/internal/SSP_MMC/SSP_MMC.h
        utils/internal/SpectralLoom/SpectralLoom.cpp
        utils/internal/SpectralLoom/SpectralLoom.h
        utils/internal/SpectralLoom/tools/ComprehensiveExamination.cpp
        utils/internal/SpectralLoom/tools/ComprehensiveExamination.h
        backend/database/DatabaseChecker.cpp
        backend/database/DatabaseChecker.h
        backend/mapper/DataInitializer.cpp
        backend/mapper/DataInitializer.h
        backend/service/MasterScheduler.cpp
        backend/service/MasterScheduler.h
)

# 假设你把那三个文件放到 external/duckdb/
target_include_directories(Bibo_no-Aozora PRIVATE
        ${CMAKE_SOURCE_DIR}/utils/external/duckdb/
)

target_link_libraries(Bibo_no-Aozora PRIVATE
        ${CMAKE_SOURCE_DIR}/utils/external/duckdb/libduckdb.dylib
)

# 运行时把 dylib 复制到可执行文件旁边（CLion/Debug 最省心）
add_custom_command(TARGET Bibo_no-Aozora POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/utils/external/duckdb/libduckdb.dylib
        $<TARGET_FILE_DIR:Bibo_no-Aozora>
)

#set_target_properties(Bibo_no-Aozora PROPERTIES
#        BUILD_RPATH "@executable_path"
#)