cmake_minimum_required(VERSION 3.16)

# Policy för att tillåta VERSION i project()
cmake_policy(SET CMP0048 NEW)

project(DarkFTP VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Quick QuickControls2 Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Quick QuickControls2 Widgets)

set(PROJECT_SOURCES
        qml_main.cpp
        qml.qrc
        src/filemodel.h
        src/filemodel.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(DarkFTP MANUAL_FINALIZATION ${PROJECT_SOURCES})

    set_target_properties(DarkFTP PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER com.example.DarkFTP
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
    )

    target_link_libraries(DarkFTP PRIVATE 
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::QuickControls2
        Qt${QT_VERSION_MAJOR}::Widgets
    )

    qt_finalize_executable(DarkFTP)

else()
    if(ANDROID)
        add_library(DarkFTP SHARED ${PROJECT_SOURCES})
    else()
        add_executable(DarkFTP ${PROJECT_SOURCES})
    endif()

    target_link_libraries(DarkFTP PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::QuickControls2
        Qt${QT_VERSION_MAJOR}::Widgets
    )
endif()
