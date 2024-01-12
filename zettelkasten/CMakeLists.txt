cmake_minimum_required(VERSION 3.16)

project(zettelkasten VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick)
find_package(yaml-cpp REQUIRED)

message("yaml-cpp: ${YAML_CPP_LIBRARY_DIR} ${YAML_CPP_LIBRARIES} ${YAML_CPP_INCLUDE_DIR}")

include_directories(${YAML_CPP_INCLUDE_DIR})

link_directories(${YAML_CPP_LIBRARY_DIR})

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(zk
    main.cpp
)

qt_add_qml_module(zk
    URI org.nettebook.zettelkasten
    VERSION 1.0
    QML_FILES
        Main.qml
        MainDelegate.qml
        PendingLink.qml
        QuadPieMenu.qml
    SOURCES
        utils.h utils.cpp
        yamldocument.h yamldocument.cpp
        zkmodel.h zkmodel.cpp
    RESOURCES
        resources/gridsquare.png
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(zk PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER org.nettebook.zettelkasten
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(zk
    PRIVATE Qt6::Quick
    ${YAML_CPP_LIBRARIES}
)

include(GNUInstallDirs)
install(TARGETS zk
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
