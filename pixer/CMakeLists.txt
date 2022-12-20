cmake_minimum_required(VERSION 2.8)

project(pixer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(WIN32)
   # set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};../SDL2-2.26.1/cmake")
   include(../../SDL2-2.26.1/cmake/sdl2-config.cmake)

else()
       find_package(SDL2 REQUIRED)
endif()

    #   find_package(jsoncpp)

    include_directories(../sc3kutils ${SDL2_INCLUDE_DIRS})

    add_executable(pixer
        main.cpp
        ../sc3kutils/log.cpp
        ../sc3kutils/log.h
		../sc3kutils/TMS9918State.cpp
        ../sc3kutils/TMS9918State.h
		../sc3kutils/TMS9918_BmConverter.cpp
        ../sc3kutils/TMS9918_BmConverter.h
    )

	# ${JSONCPP_LIBRARIES}
	# target_link_libraries(sc3kpetscii jsoncpp )
	target_link_libraries(pixer ${SDL2_LIBRARIES})
