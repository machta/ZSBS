cmake_minimum_required(VERSION 3.1)
project(Alenka)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(DEBUG true)
endif()

# Options.
set(CMAKE_CXX_STANDARD 11)

if(UNIX)
	add_definitions(-DUNIX_BUILD)

	if(DEBUG)
		add_definitions(-D_GLIBCXX_DEBUG)
	endif()
elseif(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP") # Parallel compilation mode for cl.exe.

	add_definitions(-DWIN_BUILD -D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS
		# This stops Microsoft's linker from needing the static library.
		-DBOOST_PROGRAM_OPTIONS_NO_LIB -DBOOST_SYSTEM_NO_LIB -DBOOST_FILESYSTEM_NO_LIB)

	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		link_directories(${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x64)
	else()
		link_directories(${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x86)
	endif()
elseif(APPLE)
	add_definitions(-DMAC_BUILD)
endif()

# Find packages.
find_package(Threads)
find_package(OpenCL)

find_package(Qt5Widgets)
find_package(Qt5WebSockets)
find_package(Qt5Charts)
find_package(Qt5Qml)
find_package(Qt5Quick)
find_package(Qt5QuickWidgets)

# Dependencies.
add_subdirectory(Alenka-File)
add_subdirectory(Alenka-Signal)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Alenka executable.
add_definitions(-DCL_USE_DEPRECATED_OPENCL_1_2_APIS)

include_directories(Alenka-File/include Alenka-Signal/include
	libraries/boost_1_63 libraries/pugixml/src libraries/eigen
	${OpenCL_INCLUDE_DIR})

file(GLOB SRC_BOOST_PO libraries/boost_1_63/libs/program_options/src/*.cpp)

set(RESOURCES
	libraries/ElkoAlenka/resources/resources.qrc
	resources/gpu-code/gpu-code.qrc
	resources/icons/icons.qrc
)

set(SRC
	src/DataModel/infotable.cpp
	src/DataModel/infotable.h
	src/DataModel/kernelcache.cpp
	src/DataModel/kernelcache.h
	src/DataModel/opendatafile.cpp
	src/DataModel/opendatafile.h
	src/DataModel/trackcodevalidator.cpp
	src/DataModel/trackcodevalidator.h
	src/DataModel/undocommandfactory.cpp
	src/DataModel/undocommandfactory.h
	src/DataModel/vitnessdatamodel.cpp
	src/DataModel/vitnessdatamodel.h
	src/Manager/codeeditdialog.cpp
	src/Manager/codeeditdialog.h
	src/Manager/eventmanager.cpp
	src/Manager/eventmanager.h
	src/Manager/eventtablemodel.cpp
	src/Manager/eventtablemodel.h
	src/Manager/eventtypemanager.cpp
	src/Manager/eventtypemanager.h
	src/Manager/eventtypetablemodel.cpp
	src/Manager/eventtypetablemodel.h
	src/Manager/filtermanager.cpp
	src/Manager/filtermanager.h
	src/Manager/filtervisualizer.cpp
	src/Manager/filtervisualizer.h
	src/Manager/manager.cpp
	src/Manager/manager.h
	src/Manager/montagemanager.cpp
	src/Manager/montagemanager.h
	src/Manager/montagetablemodel.cpp
	src/Manager/montagetablemodel.h
	src/Manager/sortproxymodel.cpp
	src/Manager/sortproxymodel.h
	src/Manager/tablemodel.cpp
	src/Manager/tablemodel.h
	src/Manager/trackmanager.cpp
	src/Manager/trackmanager.h
	src/Manager/tracktablemodel.cpp
	src/Manager/tracktablemodel.h
	src/SignalProcessor/lrucache.h
	src/SignalProcessor/signalprocessor.cpp
	src/SignalProcessor/signalprocessor.h
	src/Sync/syncclient.cpp
	src/Sync/syncclient.h
	src/Sync/syncdialog.cpp
	src/Sync/syncdialog.h
	src/Sync/syncserver.cpp
	src/Sync/syncserver.h
	src/canvas.cpp
	src/canvas.h
	src/error.cpp
	src/error.h
	src/main.cpp
	src/myapplication.cpp
	src/myapplication.h
	src/openglinterface.cpp
	src/openglinterface.h
	src/openglprogram.cpp
	src/openglprogram.h
	src/options.cpp
	src/options.h
	src/signalfilebrowserwindow.cpp
	src/signalfilebrowserwindow.h
	src/signalviewer.cpp
	src/signalviewer.h
	src/spikedetanalysis.cpp
	src/spikedetanalysis.h
	src/spikedetsettingsdialog.cpp
	src/spikedetsettingsdialog.h
	src/tracklabelbar.cpp
	src/tracklabelbar.h
)

add_executable(Alenka ${SRC} ${RESOURCES} ${SRC_BOOST_PO}
	libraries/spikedet/src/CResultsModel.cpp)

target_link_libraries(Alenka ${LIBS_TO_LINK_ALENKA_FILE} ${LIBS_TO_LINK_ALENKA_SIGNAL}
	${CMAKE_THREAD_LIBS_INIT} ${OpenCL_LIBRARIES}
	Qt5::Widgets Qt5::WebSockets Qt5::Charts Qt5::Qml Qt5::Quick Qt5::QuickWidgets)

add_custom_command(TARGET Alenka POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:Alenka>/log)

if(UNIX AND NOT APPLE)
	target_link_libraries(Alenka dl GL
		# Link these libraries staticaly so that the binary can be used on Ubuntu 14 as well.
		/usr/lib/gcc/x86_64-linux-gnu/5/libstdc++.a
		/usr/lib/gcc/x86_64-linux-gnu/5/libgomp.a
		/usr/lib/x86_64-linux-gnu/libmatio.a
		/usr/lib/x86_64-linux-gnu/libhdf5_serial.a
		/usr/lib/x86_64-linux-gnu/libsz.a
		/usr/lib/x86_64-linux-gnu/libaec.a
		/usr/lib/x86_64-linux-gnu/libz.a)
elseif(WIN32)
	include_directories(libraries/matio/src libraries/matio/visual_studio)
	target_link_libraries(Alenka opengl32)

	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		target_link_libraries(Alenka ${Alenka-File_SOURCE_DIR}/matio/matio-msvc2015/x64/matio.lib)

		add_custom_command(TARGET Alenka POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x64/hdf5.dll
			${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x64/libmatio.dll
			${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x64/szip.dll
			${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x64/zlib.dll
			$<TARGET_FILE_DIR:Alenka>)
	else()
		target_link_libraries(Alenka ${Alenka-File_SOURCE_DIR}/matio/matio-msvc2015/x86/matio.lib)

		add_custom_command(TARGET Alenka POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x86/hdf5.dll
			${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x86/libmatio.dll
			${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x86/szip.dll
			${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x86/zlib.dll
			$<TARGET_FILE_DIR:Alenka>)
	endif()
elseif(APPLE)
endif()

# Tests.
option(BUILD_TESTS_ALENKA "Builds unit-tests for Alenka." OFF)
set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTORCC OFF)

if(BUILD_TESTS_ALENKA)
	add_subdirectory(unit-test)
endif()