# LogReader Test Suite CMake Configuration
# Author: GeziP
# Date: 2025-06-27
# Description: CMake configuration for automated testing

cmake_minimum_required(VERSION 3.16)

# 启用测试
enable_testing()

# 查找Qt测试模块
find_package(Qt6 COMPONENTS Test QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Test REQUIRED)
endif()

# 设置测试公共配置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含项目源代码目录
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/core)
include_directories(${CMAKE_SOURCE_DIR}/src/ui)
include_directories(${CMAKE_SOURCE_DIR}/src/utils)

# 测试辅助宏
macro(add_logviewer_test testname)
    add_executable(${testname} ${testname}.cpp)
    
    if(Qt6_FOUND)
        target_link_libraries(${testname} Qt6::Test Qt6::Core Qt6::Widgets)
    else()
        target_link_libraries(${testname} Qt5::Test Qt5::Core Qt5::Widgets)
    endif()
    
    # 链接项目源文件（根据需要）
    target_sources(${testname} PRIVATE ${ARGN})
    
    # 添加到测试套件
    add_test(NAME ${testname} COMMAND ${testname})
    
    # 设置测试超时（30秒）
    set_tests_properties(${testname} PROPERTIES TIMEOUT 30)
endmacro()

# 核心功能测试
add_logviewer_test(test_logentry 
    ${CMAKE_SOURCE_DIR}/src/core/logentry.h
)

add_logviewer_test(test_logexporter 
    ${CMAKE_SOURCE_DIR}/src/core/logexporter.cpp
    ${CMAKE_SOURCE_DIR}/src/core/logexporter.h
)

# 工具类测试
add_logviewer_test(test_appsettings 
    ${CMAKE_SOURCE_DIR}/src/utils/appsettings.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/appsettings.h
)

add_logviewer_test(test_languagemanager 
    ${CMAKE_SOURCE_DIR}/src/utils/languagemanager.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/languagemanager.h
)

# UI测试（可选，可能需要display）
if(NOT $ENV{CI})  # 在CI环境中跳过UI测试
    add_logviewer_test(test_logviewer 
        ${CMAKE_SOURCE_DIR}/src/ui/logviewer.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/logviewer.h
        ${CMAKE_SOURCE_DIR}/src/core/logentry.h
        ${CMAKE_SOURCE_DIR}/src/core/logexporter.cpp
        ${CMAKE_SOURCE_DIR}/src/core/logexporter.h
        ${CMAKE_SOURCE_DIR}/src/utils/appsettings.cpp
        ${CMAKE_SOURCE_DIR}/src/utils/appsettings.h
        ${CMAKE_SOURCE_DIR}/src/utils/languagemanager.cpp
        ${CMAKE_SOURCE_DIR}/src/utils/languagemanager.h
    )
    
    add_logviewer_test(test_exportdialog 
        ${CMAKE_SOURCE_DIR}/src/ui/exportdialog.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/exportdialog.h
        ${CMAKE_SOURCE_DIR}/src/core/logentry.h
        ${CMAKE_SOURCE_DIR}/src/utils/appsettings.cpp
        ${CMAKE_SOURCE_DIR}/src/utils/appsettings.h
    )
endif()

# 集成测试
add_logviewer_test(test_integration)

# 性能测试
add_logviewer_test(test_performance 
    ${CMAKE_SOURCE_DIR}/src/core/logexporter.cpp
    ${CMAKE_SOURCE_DIR}/src/core/logexporter.h
)

# 自定义测试目标
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --verbose
    DEPENDS 
        test_logentry 
        test_logexporter 
        test_appsettings 
        test_languagemanager 
        test_integration 
        test_performance
    COMMENT "Running all LogReader tests"
)

# 测试覆盖率（如果启用）
if(ENABLE_COVERAGE)
    include(CodeCoverage)
    setup_target_for_coverage_gcovr_html(
        NAME coverage
        EXECUTABLE run_tests
        DEPENDENCIES run_tests
        BASE_DIRECTORY "${CMAKE_SOURCE_DIR}"
        EXCLUDE "tests/*" "build/*" "/usr/*"
    )
endif()

message(STATUS "LogReader test suite configured successfully")
message(STATUS "Available test targets:")
message(STATUS "  - test_logentry: Core data structure tests")
message(STATUS "  - test_logexporter: Export functionality tests")
message(STATUS "  - test_appsettings: Settings management tests")
message(STATUS "  - test_languagemanager: Multi-language support tests")
message(STATUS "  - test_integration: End-to-end integration tests")
message(STATUS "  - test_performance: Performance and load tests")
message(STATUS "  - run_tests: Run all tests")
if(ENABLE_COVERAGE)
    message(STATUS "  - coverage: Generate code coverage report")
endif() 