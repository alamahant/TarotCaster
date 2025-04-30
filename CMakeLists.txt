cmake_minimum_required(VERSION 3.16)

project(TarotCaster VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Qt6Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)
#configure_file(card_meanings.json ${CMAKE_BINARY_DIR} COPYONLY)
configure_file(resources/card_meanings.json ${CMAKE_BINARY_DIR} COPYONLY)


file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/decks DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h

)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)


qt_add_executable(TarotCaster
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        cardloader.h cardloader.cpp
        tarotscene.h tarotscene.cpp
        tarotcarditem.h tarotcarditem.cpp
        cardmeaning.h cardmeaning.cpp
        meaningdisplay.h meaningdisplay.cpp
        dockcontrols.h dockcontrols.cpp
        mistralapi.h mistralapi.cpp
        resources.qrc
        helpdialog.h helpdialog.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET TarotCaster APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(TarotCaster SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(TarotCaster
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(TarotCaster PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt6::Network)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.TarotCaster)
endif()
set_target_properties(TarotCaster PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS TarotCaster
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(TarotCaster)
endif()

# Install desktop file
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/io.github.alamahant.TarotCaster.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

# Install metainfo file
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/io.github.alamahant.TarotCaster.metainfo.xml
        DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)

# Install icon
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/io.github.alamahant.TarotCaster.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/512x512/apps)

# Install data files
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/decks/OriginalRiderWaite
        DESTINATION ${CMAKE_INSTALL_BINDIR}/decks)

      install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/decks/TarotdeMarseille
              DESTINATION ${CMAKE_INSTALL_BINDIR}/decks)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/card_meanings.json
        DESTINATION ${CMAKE_INSTALL_BINDIR})
