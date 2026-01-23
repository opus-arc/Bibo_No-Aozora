cmake_minimum_required(VERSION 4.0)
project(Bibo_no-Aozora)

set(CMAKE_CXX_STANDARD 20)


add_executable(
        Bibo_no-Aozora main.cpp
        SpectralLoom/tools/Pitch.cpp
        SpectralLoom/tools/Pitch.h
        external/miniaudio.h
        SpectralLoom/tools/RecordPlayer.cpp
        SpectralLoom/tools/RecordPlayer.h
        SpectralLoom/tools/SingleTrack.h
        SpectralLoom/tools/SingleTrack.cpp
        SpectralLoom/tools/Chord.cpp
        SpectralLoom/tools/Chord.h
        SpectralLoom/tools/EnvHarmonics.cpp
        SpectralLoom/tools/EnvHarmonics.h
        SpectralLoom/tools/CelloDynamic.cpp
        SpectralLoom/tools/CelloDynamic.h
        SSP_MMC/SSP_MMC.cpp
        SSP_MMC/SSP_MMC.h
        SpectralLoom/SpectralLoom.cpp
        SpectralLoom/SpectralLoom.h
        SpectralLoom/tools/ComprehensiveExamination.cpp
        SpectralLoom/tools/ComprehensiveExamination.h
)
