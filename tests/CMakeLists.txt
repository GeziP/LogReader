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

    # 包含项目源代码目录
    target_include_directories(${testname} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/core
        ${CMAKE_SOURCE_DIR}/src/ui
        ${CMAKE_SOURCE_DIR}/src/utils
    )

    # 添加到测试套件
    add_test(NAME ${testname} COMMAND ${testname})
    
    # 设置测试超时（30秒）
    set_tests_properties(${testname} PROPERTIES TIMEOUT 30)
    
    # 为覆盖率测试添加环境变量
    if(ENABLE_COVERAGE)
        set_tests_properties(${testname} PROPERTIES
            ENVIRONMENT "GCOV_PREFIX=${CMAKE_BINARY_DIR}"
        )
    endif()
endmacro()

# 核心功能测试（仅构建存在的测试文件）
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_logentry.cpp")
    add_logviewer_test(test_logentry 
        ${CMAKE_SOURCE_DIR}/src/core/logentry.h
    )
    set(TEST_TARGETS test_logentry)
endif()

# 以下测试文件暂未实现，注释掉避免CMake错误
# if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_logexporter.cpp")
#     add_logviewer_test(test_logexporter 
#         ${CMAKE_SOURCE_DIR}/src/core/logexporter.cpp
#         ${CMAKE_SOURCE_DIR}/src/core/logexporter.h
#     )
#     list(APPEND TEST_TARGETS test_logexporter)
# endif()

# if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_appsettings.cpp")
#     add_logviewer_test(test_appsettings 
#         ${CMAKE_SOURCE_DIR}/src/utils/appsettings.cpp
#         ${CMAKE_SOURCE_DIR}/src/utils/appsettings.h
#     )
#     list(APPEND TEST_TARGETS test_appsettings)
# endif()

# if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_languagemanager.cpp")
#     add_logviewer_test(test_languagemanager 
#         ${CMAKE_SOURCE_DIR}/src/utils/languagemanager.cpp
#         ${CMAKE_SOURCE_DIR}/src/utils/languagemanager.h
#     )
#     list(APPEND TEST_TARGETS test_languagemanager)
# endif()

# UI测试（可选，可能需要display）
if(NOT $ENV{CI})  # 在CI环境中跳过UI测试
    # if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_logviewer.cpp")
    #     add_logviewer_test(test_logviewer 
    #         ${CMAKE_SOURCE_DIR}/src/ui/logviewer.cpp
    #         ${CMAKE_SOURCE_DIR}/src/ui/logviewer.h
    #         ${CMAKE_SOURCE_DIR}/src/core/logentry.h
    #         ${CMAKE_SOURCE_DIR}/src/core/logexporter.cpp
    #         ${CMAKE_SOURCE_DIR}/src/core/logexporter.h
    #         ${CMAKE_SOURCE_DIR}/src/utils/appsettings.cpp
    #         ${CMAKE_SOURCE_DIR}/src/utils/appsettings.h
    #         ${CMAKE_SOURCE_DIR}/src/utils/languagemanager.cpp
    #         ${CMAKE_SOURCE_DIR}/src/utils/languagemanager.h
    #     )
    #     list(APPEND TEST_TARGETS test_logviewer)
    # endif()
    
    # if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_exportdialog.cpp")
    #     add_logviewer_test(test_exportdialog 
    #         ${CMAKE_SOURCE_DIR}/src/ui/exportdialog.cpp
    #         ${CMAKE_SOURCE_DIR}/src/ui/exportdialog.h
    #         ${CMAKE_SOURCE_DIR}/src/core/logentry.h
    #         ${CMAKE_SOURCE_DIR}/src/utils/appsettings.cpp
    #         ${CMAKE_SOURCE_DIR}/src/utils/appsettings.h
    #     )
    #     list(APPEND TEST_TARGETS test_exportdialog)
    # endif()
endif()

# 其他测试（暂未实现）
# if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_integration.cpp")
#     add_logviewer_test(test_integration)
#     list(APPEND TEST_TARGETS test_integration)
# endif()

# if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_performance.cpp")
#     add_logviewer_test(test_performance 
#         ${CMAKE_SOURCE_DIR}/src/core/logexporter.cpp
#         ${CMAKE_SOURCE_DIR}/src/core/logexporter.h
#     )
#     list(APPEND TEST_TARGETS test_performance)
# endif()

# 自定义测试目标（只依赖存在的测试）
if(DEFINED TEST_TARGETS)
    add_custom_target(run_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --verbose
        DEPENDS ${TEST_TARGETS}
        COMMENT "Running all LogReader tests"
    )
else()
    # 如果没有测试文件，创建一个空的目标
    add_custom_target(run_tests
        COMMAND ${CMAKE_COMMAND} -E echo "No tests available to run"
        COMMENT "No tests configured"
    )
endif()

# 简化的Coverage目标
if(ENABLE_COVERAGE)
    find_program(GCOVR_EXECUTABLE gcovr)
    find_program(LCOV_EXECUTABLE lcov)
    
    if(LCOV_EXECUTABLE)
        add_custom_target(coverage
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
            COMMAND ${LCOV_EXECUTABLE} --capture --directory ${CMAKE_BINARY_DIR} --output-file ${CMAKE_BINARY_DIR}/coverage/coverage.info
            COMMAND ${LCOV_EXECUTABLE} --remove ${CMAKE_BINARY_DIR}/coverage/coverage.info '/usr/*' '*/tests/*' '*/build/*' --output-file ${CMAKE_BINARY_DIR}/coverage/coverage_filtered.info
            COMMAND ${LCOV_EXECUTABLE} --list ${CMAKE_BINARY_DIR}/coverage/coverage_filtered.info
            DEPENDS run_tests
            COMMENT "Generating coverage report"
        )
    elseif(GCOVR_EXECUTABLE)
        add_custom_target(coverage
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
            COMMAND ${GCOVR_EXECUTABLE} --root ${CMAKE_SOURCE_DIR} --html --html-details -o ${CMAKE_BINARY_DIR}/coverage/coverage.html ${CMAKE_BINARY_DIR}
            DEPENDS run_tests
            COMMENT "Generating coverage report with gcovr"
        )
    endif()
endif()

message(STATUS "LogReader test suite configured successfully")
message(STATUS "Available test targets:")
if(DEFINED TEST_TARGETS)
    foreach(target ${TEST_TARGETS})
        message(STATUS "  - ${target}")
    endforeach()
else()
    message(STATUS "  - No test files found")
endif()
message(STATUS "  - run_tests: Run all tests")
if(ENABLE_COVERAGE)
    message(STATUS "  - coverage: Generate code coverage report")
endif() 