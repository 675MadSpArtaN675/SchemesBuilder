cmake_minimum_required(VERSION 3.16)

project(GraphPainter VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB HEADER_FILES "./*.hpp" "./*.h" )
file(GLOB SOURCES_FILES "./*.cpp")

find_package(Qt6 REQUIRED COMPONENTS Quick)
find_package(Boost REQUIRED)
find_package(DataFrame REQUIRED)

qt_standard_project_setup(REQUIRES 6.10)

qt_add_executable(appGraphPainter
    ${SOURCES_FILES}
    ${HEADER_FILES}
)

qt_add_qml_module(appGraphPainter
    URI GraphPainter
    QML_FILES
        Main.qml
        TableLoadDialog.qml
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appGraphPainter PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appGraphPainter
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appGraphPainter
    PRIVATE Qt6::Quick DataFrame
)

target_include_directories(appGraphPainter PRIVATE ${Boost_INCLUDE_DIR} ${DataFrame_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS appGraphPainter
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
