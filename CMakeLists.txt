cmake_minimum_required(VERSION 3.15)
project(LogReader VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Core Gui Widgets LinguistTools REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB ts ${CMAKE_CURRENT_SOURCE_DIR}/translations/*)
qt_add_translation(QM_FILES ${ts})
add_custom_target(translations ALL DEPENDS ${QM_FILES})

file(GLOB SOURCES *.cpp)
file(GLOB HEADERS *.h)
file(GLOB RESOURCE_FILE *.qrc)
file(GLOB UI_FILES *.ui)
file(GLOB RESOURCES icons/*)

add_executable(LogReader ${SOURCES} ${HEADERS} ${RESOURCE_FILE} ${UI_FILES} ${TS_FILES} ${RESOURCES})

target_include_directories(LogReader PRIVATE ${PROJECT_SOURCE_DIR})

target_link_libraries(LogReader Qt6::Core Qt6::Gui Qt6::Widgets)

get_target_property(_qt5_qmake_location Qt::qmake IMPORTED_LOCATION)

execute_process(
    COMMAND "${_qt5_qmake_location}" -query QT_INSTALL_PREFIX
    RESULT_VARIABLE return_code
    OUTPUT_VARIABLE qt5_install_prefix
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(imported_location "${qt5_install_prefix}/bin/windeployqt.exe")
add_custom_command(TARGET LogReader
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory "$$<TARGET_FILE_DIR:LogReader>"
    COMMAND ${imported_location} --dir "$<TARGET_FILE_DIR:LogReader>" "$<TARGET_FILE_DIR:LogReader>/$<TARGET_FILE_NAME:LogReader>"
    COMMENT "Deploy Qt libraries"
)
