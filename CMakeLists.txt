cmake_minimum_required(VERSION 3.20)
project(Bibo_no-Aozora)

set(CMAKE_CXX_STANDARD 20)

add_executable(
        Bibo_no-Aozora main.cpp
        Src/Pitch.cpp
        Src/Pitch.h
        External/miniaudio.h
        Src/RecordPlayer.cpp
        Src/RecordPlayer.h
        Src/SingleTrack.h
        Src/SingleTrack.cpp
        Src/Chord.cpp
        Src/Chord.h
        Src/EnvHarmonics.cpp
        Src/EnvHarmonics.h
)
