cmake_minimum_required(VERSION 2.8)

project(pixer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(WIN32)
   include(../../SDL2-2.26.1/cmake/sdl2-config.cmake)
   include(../../SDL2_image-2.6.2/cmake/sdl2_image-config.cmake)
   set(SDL2_IMAGE_INCLUDE_DIRS "../../SDL2_image-2.6.2/include")
   set(SDL2_IMAGE_LIBRARIES SDL2_image::SDL2_image)
else()
    find_package(SDL2 REQUIRED)
#    find_package(SDL2_image REQUIRED)
endif()

    #   find_package(jsoncpp)
    # message( STATUS "**WOOT:${SDL2_INCLUDE_DIRS}")
    include_directories(../sc3kutils ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} )

    add_executable(pixer
        main.cpp
        ../sc3kutils/log.cpp
        ../sc3kutils/log.h
		../sc3kutils/TMS9918State.cpp
        ../sc3kutils/TMS9918State.h
		../sc3kutils/TMS9918_BmConverter.cpp
        ../sc3kutils/TMS9918_BmConverter.h
        ../sc3kutils/TMS9918_SC2Loader.cpp
        ../sc3kutils/TMS9918_SC2Loader.h
        ../sc3kutils/TMS9918_SC3KExport.cpp
        ../sc3kutils/TMS9918_SC3KExport.h
    )

	# ${JSONCPP_LIBRARIES}
	# target_link_libraries(sc3kpetscii jsoncpp )
	target_link_libraries(pixer ${SDL2_LIBRARIES}  ${SDL2_IMAGE_LIBRARIES} )
