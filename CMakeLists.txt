cmake_minimum_required(VERSION 3.13 FATAL_ERROR) # target_link_directories

SET(CMAKE_BUILD_TYPE MinSizeRel)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

project(tray C)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Generate 'compile_commands.json' for clang_complete
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB SRCS
        ${CMAKE_CURRENT_LIST_DIR}/*.h
        ${CMAKE_CURRENT_LIST_DIR}/cJSON.c
        ${CMAKE_CURRENT_LIST_DIR}/*.ico
        ${CMAKE_CURRENT_LIST_DIR}/*.png)

if(WIN32)
	list(APPEND SRCS ${CMAKE_CURRENT_SOURCE_DIR}/tray_windows.c)
else()
	if(UNIX)
		if(APPLE)
			find_library(COCOA Cocoa REQUIRED)
			list(APPEND SRCS ${CMAKE_CURRENT_SOURCE_DIR}/tray_darwin.m)
		else()
			FIND_PACKAGE(PkgConfig)
			PKG_CHECK_MODULES(APPINDICATOR REQUIRED ayatana-appindicator3-0.1)
			list(APPEND SRCS ${CMAKE_CURRENT_SOURCE_DIR}/tray_linux.c)
		endif()
	endif()
endif()

add_library(tray STATIC ${SRCS})
set_property(TARGET tray PROPERTY C_STANDARD 99)
if(WIN32)
	target_compile_definitions(tray PRIVATE TRAY_WINAPI=1 WIN32_LEAN_AND_MEAN NOMINMAX)
    set(FILE_OUTPUT_NAME ideckia_tray)
else()
        if(UNIX)
            if(APPLE)
                target_compile_definitions(tray PRIVATE TRAY_APPKIT=1)
                target_link_libraries(tray PRIVATE ${COCOA})
                set(FILE_OUTPUT_NAME ideckia_tray_macos)
            else()
                target_compile_options(tray PRIVATE ${APPINDICATOR_CFLAGS})
                target_link_directories(tray PRIVATE ${APPINDICATOR_LIBRARY_DIRS})
                target_compile_definitions(tray PRIVATE TRAY_APPINDICATOR=1)
                target_link_libraries(tray PRIVATE ${APPINDICATOR_LIBRARIES})
                set(FILE_OUTPUT_NAME ideckia_tray_linux)
            endif()
        endif()
endif()

add_library(tray::tray ALIAS tray)

add_executable(tray_app ${CMAKE_CURRENT_SOURCE_DIR}/main.c)
set_target_properties(
    tray_app
    PROPERTIES 
        OUTPUT_NAME ${FILE_OUTPUT_NAME}
)
target_link_libraries(tray_app tray::tray)

INSTALL(TARGETS tray tray DESTINATION lib)

IF(NOT WIN32)
	INSTALL(FILES tray.h DESTINATION include)
ENDIF()
