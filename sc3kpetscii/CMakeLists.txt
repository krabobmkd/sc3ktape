cmake_minimum_required(VERSION 2.8)

project(sc3kpetscii LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

    find_package(jsoncpp)
    include_directories(../sc3kutils)

    add_executable(sc3kpetscii
        main.cpp
        ../sc3kutils/log.cpp
        ../sc3kutils/log.h
        ../sc3kutils/SegaAsciiJpToUnicode.h
        ../sc3kutils/PetsciiToUnicode.h
    )

# ${JSONCPP_LIBRARIES}
    target_link_libraries(sc3kpetscii jsoncpp )

    message(STATUS " ____   ___  ____  __ _  ____  ____  ____  ____   ___  __  __")
    message(STATUS "/ ___) / __)( __ \\(  / )(  _ \\(  __)(_  _)/ ___) / __)(  )(  )")
    message(STATUS "\\___ \\( (__  (__ ( )  (  ) __/ ) _)   )(  \\___ \\( (__  )(  )(")
    message(STATUS "(____/ \\___)(____/(__\\_)(__)  (____) (__) (____/ \\___)(__)(__)")
    message(STATUS ".oO petmate jso PETSCII screen to Sega SC3000 Basic Converter Oo.")
    message(STATUS "   ")
