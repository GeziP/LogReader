cmake_minimum_required(VERSION 3.15)
project(LogReader VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 尝试查找Qt6，如果找不到则查找Qt5
find_package(Qt6 COMPONENTS Core Gui Widgets LinguistTools QUIET)
if (NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Gui Widgets LinguistTools QUIET)
    if (NOT Qt5_FOUND)
        message(FATAL_ERROR "Neither Qt6 nor Qt5 found. Please install Qt5 or Qt6, or use qmake with LogReader.pro file.")
    endif()
    set(QT_VERSION_MAJOR 5)
else()
    set(QT_VERSION_MAJOR 6)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 设置翻译文件
set(TS_FILES
    translations/translation_zh_CN.ts
    translations/translation_en.ts
)

# 根据Qt版本使用不同的翻译命令
if(QT_VERSION_MAJOR EQUAL 6)
    qt_add_translation(QM_FILES ${TS_FILES})
else()
    qt5_add_translation(QM_FILES ${TS_FILES})
endif()

# 创建翻译目标
add_custom_target(translations ALL DEPENDS ${QM_FILES})

# 确保翻译文件被复制到输出目录
foreach(qm_file ${QM_FILES})
    get_filename_component(qm_filename ${qm_file} NAME)
    add_custom_command(TARGET LogReader POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:LogReader>/translations"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${qm_file}" "$<TARGET_FILE_DIR:LogReader>/translations/${qm_filename}"
        COMMENT "Copying ${qm_filename} to output directory"
    )
endforeach()

# 确保翻译在主程序之前编译
add_dependencies(LogReader translations)

file(GLOB SOURCES 
    src/*.cpp 
    src/ui/*.cpp 
    src/core/*.cpp 
    src/utils/*.cpp)
file(GLOB HEADERS 
    src/*.h 
    src/ui/*.h 
    src/core/*.h 
    src/utils/*.h)
file(GLOB RESOURCE_FILE resources/*.qrc)
file(GLOB UI_FILES src/ui/*.ui)
file(GLOB RESOURCES resources/icons/*)

add_executable(LogReader ${SOURCES} ${HEADERS} ${RESOURCE_FILE} ${UI_FILES} ${TS_FILES} ${RESOURCES})

target_include_directories(LogReader PRIVATE 
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/ui
    ${PROJECT_SOURCE_DIR}/src/core
    ${PROJECT_SOURCE_DIR}/src/utils)

# 根据Qt版本链接不同的库
if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(LogReader Qt6::Core Qt6::Gui Qt6::Widgets)
else()
    target_link_libraries(LogReader Qt5::Core Qt5::Gui Qt5::Widgets)
endif()

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
