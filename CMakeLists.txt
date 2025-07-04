cmake_minimum_required(VERSION 3.15)
project(LogReader VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 构建选项
option(ENABLE_TESTING "Build tests" OFF)
option(ENABLE_COVERAGE "Enable coverage" OFF)

# Coverage配置
if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    endif()
endif()

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

# 收集源文件
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

# 创建可执行文件（必须在使用目标之前定义）
# 添加WIN32标志隐藏控制台窗口
add_executable(LogReader WIN32 ${SOURCES} ${HEADERS} ${RESOURCE_FILE} ${UI_FILES} ${TS_FILES})

# 设置包含目录
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

# 创建翻译目标
add_custom_target(translations ALL DEPENDS ${QM_FILES})

# 确保翻译在主程序之前编译
add_dependencies(LogReader translations)

# 确保翻译文件被复制到输出目录
foreach(qm_file ${QM_FILES})
    get_filename_component(qm_filename ${qm_file} NAME)
    add_custom_command(TARGET LogReader POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:LogReader>/translations"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${qm_file}" "$<TARGET_FILE_DIR:LogReader>/translations/${qm_filename}"
        COMMENT "Copying ${qm_filename} to output directory"
    )
endforeach()

# 测试支持
if(ENABLE_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# 可选：Windows部署命令（仅在找到windeployqt时执行）
if(WIN32 AND QT_VERSION_MAJOR EQUAL 6)
    get_target_property(_qt_qmake_location Qt6::qmake IMPORTED_LOCATION)
elseif(WIN32 AND QT_VERSION_MAJOR EQUAL 5)
    get_target_property(_qt_qmake_location Qt5::qmake IMPORTED_LOCATION)
endif()

if(WIN32 AND _qt_qmake_location)
    execute_process(
        COMMAND "${_qt_qmake_location}" -query QT_INSTALL_PREFIX
        RESULT_VARIABLE return_code
        OUTPUT_VARIABLE qt_install_prefix
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    set(windeployqt_location "${qt_install_prefix}/bin/windeployqt.exe")
    if(EXISTS "${windeployqt_location}")
        add_custom_command(TARGET LogReader POST_BUILD
            COMMAND "${windeployqt_location}" --release --no-translations "$<TARGET_FILE:LogReader>"
            COMMENT "Deploy Qt libraries"
        )
    endif()
endif()
