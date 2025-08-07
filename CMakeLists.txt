cmake_minimum_required(VERSION 3.12)
project(ptxasm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 配置 ANTLR ---
# 设置 ANTLR 可执行文件路径 (需要根据你的安装情况修改)
set(ANTLR_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/antlr4/antlr-4.13.2-complete.jar)
set(ANTLR_RUNTIME_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/antlr4/runtime/src)

# 检查 ANTLR 是否存在
if(NOT EXISTS ${ANTLR_EXECUTABLE})
    message(FATAL_ERROR "ANTLR executable not found at ${ANTLR_EXECUTABLE}. Please set the correct path in CMakeLists.txt.")
endif()

# 设置 Java 命令 (ANTLR 需要 Java)
find_package(Java REQUIRED)
include(UseJava)

# --- 定义 ANTLR 生成 PTX 解析器 ---
set(ANTLR_JAR ${ANTLR_EXECUTABLE})
set(ANTLR_PTX_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/antlr_generated_ptx)
file(MAKE_DIRECTORY ${ANTLR_PTX_GENERATED_DIR})

set(GRAMMAR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/grammar)
set(PTX_LEXER_FILE ${GRAMMAR_DIR}/ptxLexer.g4)
set(PTX_PARSER_FILE ${GRAMMAR_DIR}/ptxParser.g4)

# ANTLR 生成的文件列表
set(ANTLR_PTX_OUTPUT_FILES
    ${ANTLR_PTX_GENERATED_DIR}/ptxLexer.cpp
    ${ANTLR_PTX_GENERATED_DIR}/ptxLexer.h
    ${ANTLR_PTX_GENERATED_DIR}/ptxParser.cpp
    ${ANTLR_PTX_GENERATED_DIR}/ptxParser.h
    ${ANTLR_PTX_GENERATED_DIR}/ptxParserBaseVisitor.cpp
    ${ANTLR_PTX_GENERATED_DIR}/ptxParserBaseVisitor.h
    ${ANTLR_PTX_GENERATED_DIR}/ptxParserVisitor.cpp
    ${ANTLR_PTX_GENERATED_DIR}/ptxParserVisitor.h
)

# 添加自定义命令来运行 ANTLR
add_custom_command(
    OUTPUT ${ANTLR_PTX_OUTPUT_FILES}
    COMMAND ${Java_JAVA_EXECUTABLE} -jar ${ANTLR_JAR}
        -Dlanguage=Cpp
        -visitor
        -no-listener # 我们使用 Visitor 模式
        -o ${ANTLR_PTX_GENERATED_DIR}
        ${PTX_LEXER_FILE} # 通常包含 Lexer 规则，所以只需指定 Parser 文件
        ${PTX_PARSER_FILE} # 通常包含 Lexer 规则，所以只需指定 Parser 文件
    DEPENDS ${PTX_LEXER_FILE} ${PTX_PARSER_FILE}
    COMMENT "Generating ANTLR C++ files for PTX..."
    VERBATIM
)

# 创建一个自定义目标来封装 ANTLR 生成
add_custom_target(antlr_gen_ptx DEPENDS ${ANTLR_PTX_OUTPUT_FILES})


# --- 定义 ANTLR 生成 COASM 解析器 ---
set(ANTLR_COASM_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/antlr_generated_coasm)
file(MAKE_DIRECTORY ${ANTLR_COASM_GENERATED_DIR})

set(COASM_GRAMMAR_FILE ${GRAMMAR_DIR}/coasm.g4) # <--- 新增

# 检查 coasm.g4 是否存在
if(NOT EXISTS ${COASM_GRAMMAR_FILE})
    message(FATAL_ERROR "COASM grammar file not found at ${COASM_GRAMMAR_FILE}. Please place coasm.g4 in the src directory.")
endif()

set(ANTLR_COASM_OUTPUT_FILES
    ${ANTLR_COASM_GENERATED_DIR}/coasmLexer.cpp
    ${ANTLR_COASM_GENERATED_DIR}/coasmLexer.h
    ${ANTLR_COASM_GENERATED_DIR}/coasmParser.cpp
    ${ANTLR_COASM_GENERATED_DIR}/coasmParser.h
    # 注意：coasm.g4 是一个 combined grammar，所以只生成 Parser 和 Lexer
    # 如果你拆分成 coasmLexer.g4 和 coasmParser.g4，则需要调整这里
)

# 添加自定义命令来运行 ANTLR 生成 coasm 解析器
add_custom_command(
    OUTPUT ${ANTLR_COASM_OUTPUT_FILES}
    COMMAND ${Java_JAVA_EXECUTABLE} -jar ${ANTLR_JAR}
        -Dlanguage=Cpp
        -visitor # 如果你需要 Visitor 模式来序列化 coasm AST
        -no-listener # 如果你不需要 Listener 模式
        -o ${ANTLR_COASM_GENERATED_DIR}
        ${COASM_GRAMMAR_FILE} # <--- 使用 coasm.g4
    DEPENDS ${COASM_GRAMMAR_FILE}
    COMMENT "Generating ANTLR C++ files for COASM..."
    VERBATIM
)

# 创建一个自定义目标来封装 COASM ANTLR 生成
add_custom_target(antlr_gen_coasm DEPENDS ${ANTLR_COASM_OUTPUT_FILES})

# --- 项目源文件 ---
set(SOURCES
    src/main.cpp
    src/PtxTranslator.cpp
    ${ANTLR_PTX_OUTPUT_FILES}   # 包含 PTX ANTLR 生成的文件
    ${ANTLR_COASM_OUTPUT_FILES} # 包含 COASM ANTLR 生成的文件 # <--- 新增
)

# --- 创建可执行文件 ---
add_executable(ptxasm ${SOURCES})

# 确保在编译 ptxasm 之前先运行 ANTLR 生成
# 现在需要同时依赖 PTX 和 COASM 的生成
add_dependencies(ptxasm antlr_gen_ptx antlr_gen_coasm) # <--- 更新依赖

# --- 设置包含目录 ---
target_include_directories(ptxasm PRIVATE
    ${ANTLR_PTX_GENERATED_DIR}   # 包含 PTX ANTLR 生成的头文件
    ${ANTLR_COASM_GENERATED_DIR} # 包含 COASM ANTLR 生成的头文件 # <--- 新增
    ${PTX_GRAMMAR_DIR}     # 包含原始 .g4 文件目录 (如果需要)
    ${COASM_GRAMMAR_DIR}         # 包含原始 .g4 文件目录 (如果需要) # <--- 新增
    ${ANTLR_RUNTIME_DIR}   # ANTLR 运行时库路径
)

# --- 链接库 ---
# 需要链接 ANTLR C++ 运行时库
# 1. 如果是通过包管理器安装的 (例如 apt install libantlr4-runtime-dev)
# find_package(antlr4-runtime REQUIRED)
# target_link_libraries(ptxasm antlr4-runtime::antlr4-runtime)

# 2. 如果是手动编译安装的，需要指定库路径和名称
# 假设库安装在 /usr/local/lib (需要根据实际情况修改)
# target_link_directories(ptxasm PRIVATE /usr/local/lib) # <--- 修改为你的 ANTLR 运行时库路径
# target_link_libraries(ptxasm antlr4-runtime)           # <--- 修改为你的 ANTLR 运行时库名称

# 3. 如果 ANTLR 运行时是作为子模块或源码构建的，需要相应地链接
# 示例 (假设 ANTLR 运行时源码在项目外部的 antlr4_runtime 目录下并已构建)
# add_subdirectory(../antlr4_runtime ${CMAKE_CURRENT_BINARY_DIR}/antlr4_runtime_build)
# target_link_libraries(ptxasm antlr4_static) # 或 antlr4_shared

# --- 替换为适合你环境的链接方式 ---
# 这里使用一个常见的假设路径和名称，你需要根据实际情况修改
target_link_directories(ptxasm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../build/antlr4/usr/local/lib)
target_link_libraries(ptxasm antlr4-runtime)

# --- 安装 (可选) ---
install(TARGETS ptxasm DESTINATION bin)

# --- Testing ---
# Enable testing functionality
enable_testing()

# Add the test executable
add_executable(test_ptxasm test/test_ptxasm.cpp)

# Ensure test_ptxasm is built after ptxasm
add_dependencies(test_ptxasm ptxasm)

# Add the test to CTest
# It will run the test_ptxasm executable
add_test(NAME test_ptxasm COMMAND test_ptxasm)

# Set properties for the test (e.g., working directory)
# This ensures the test runs in the 'test' directory where the .ptx files are
set_property(TEST test_ptxasm PROPERTY WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test)

# --- Optional: Custom test target ---
# You can also create a custom target to make running tests easier
# add_custom_target(run_tests
#     COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
#     DEPENDS test_ptxasm
#     WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
# )
# This allows running 'make run_tests' or 'cmake --build . --target run_tests'
