cmake_minimum_required(VERSION 4.0)
project(Bibo_no-Aozora)

set(CMAKE_CXX_STANDARD 20)


add_executable(
        Bibo_no-Aozora main.cpp
        SpectralLoom/Pitch.cpp
        SpectralLoom/Pitch.h
        external/miniaudio.h
        SpectralLoom/RecordPlayer.cpp
        SpectralLoom/RecordPlayer.h
        SpectralLoom/SingleTrack.h
        SpectralLoom/SingleTrack.cpp
        SpectralLoom/Chord.cpp
        SpectralLoom/Chord.h
        SpectralLoom/EnvHarmonics.cpp
        SpectralLoom/EnvHarmonics.h
        SpectralLoom/CelloDynamic.cpp
        SpectralLoom/CelloDynamic.h
        SSP_MMC/SSP_MMC.cpp
        SSP_MMC/SSP_MMC.h
)
