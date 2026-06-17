cmake_minimum_required(VERSION 3.16)

project(GraphPainter VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build test?" OFF)

file(GLOB HEADER_FILES "*.hpp" "*.h" )
file(GLOB SOURCES_FILES "*.cpp")
file(GLOB TEST_FILES "*_test.cpp")

list(FILTER SOURCES_FILES EXCLUDE REGEX ".*_test.cpp")

find_package(Qt6 REQUIRED COMPONENTS Quick Core QuickWidgets)
find_package(Boost REQUIRED headers log log_setup regex)
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
        GraphField.qml
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
    PRIVATE Qt6::Quick Qt6::Core Qt6::QuickWidgets DataFrame Boost::headers Boost::log Boost::log_setup Boost::regex
)

target_include_directories(appGraphPainter PRIVATE ${Boost_INCLUDE_DIR} ${DataFrame_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(appGraphPainter PRIVATE BOOST_REGEX_MATCH_EXTRA BOOST_BIND_GLOBAL_PLACEHOLDERS)

message("Option \'BUILD_TESTS\' = ${BUILD_TESTS}")
if(BUILD_TESTS)
    message("Building tests")

    set(SOURCES_FILES_FOR_TESTS ${SOURCES_FILES})
    list(FILTER SOURCES_FILES_FOR_TESTS EXCLUDE REGEX ".*main.cpp")

    add_executable(tests ${TEST_FILES} ${SOURCES_FILES_FOR_TESTS})

    target_include_directories(tests PRIVATE ".")
    target_link_libraries(tests PRIVATE Qt6::Quick Qt6::Core Qt6::QuickWidgets DataFrame Boost::headers Boost::log Boost::log_setup)

    message("Tests builded")
endif()

install(TARGETS appGraphPainter
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
