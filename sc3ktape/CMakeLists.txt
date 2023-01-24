cmake_minimum_required(VERSION 2.8)

project(sc3ktape LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

    include_directories(../sc3kutils)

    add_executable(sc3ktape
        main.cpp
        ../sc3kutils/log.cpp
        ../sc3kutils/log.h
        SC3KBasic.cpp
        SC3KBasic.h
        SC3KSoundAndBytes.cpp
        SC3KSoundAndBytes.h
        SoundReader.cpp
        SoundReader.h
        ../sc3kutils/SegaAsciiJpToUnicode.h
    )
    add_executable(mekastateloader
        mekastateloader.cpp
    )

    message(STATUS "              ________  __     __")
    message(STATUS "  ______ ____ \\_____  \\|  | __/  |______  ______   ____ ")
    message(STATUS " /  ____/ ___\\  _(__  <|  |/ \\   __\\__  \\ \\____ \\_/ __ \\")
    message(STATUS " \\___ \\\\  \\___ /       |    < |  |  / __ \\|  |_> \\  ___/")
    message(STATUS "/____  >\\___  /______  |__|_ \\|__| (____  |   __/ \\___  >")
    message(STATUS "     \\/     \\/       \\/     \\/          \\/|__|        \\/")
    message(STATUS " .oO Sega SC3000/SK1100 Tape-2-Basic-2-Tape Converter Oo.")
    message(STATUS "   ")
