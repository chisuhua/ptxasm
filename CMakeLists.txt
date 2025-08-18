cmake_minimum_required(VERSION 3.12)
project(ptxasm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INSTALL_PREFIX "" CACHE PATH "")
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")

# --- 配置 ANTLR ---
set(ANTLR_JAR $ENV{ANTLR_JAR})
set(ANTLR_RUNTIME_DIR $ENV{ANTLR_RUNTIME_DIR})

if(NOT EXISTS ${ANTLR_JAR})
    message(FATAL_ERROR "ANTLR executable not found at ${ANTLR_JAR}. Please set the correct path in CMakeLists.txt.")
endif()

if(NOT EXISTS ${ANTLR_RUNTIME_DIR})
    message(FATAL_ERROR "ANTLR executable not found at ${ANTLR_JAR}. Please set the correct path in CMakeLists.txt.")
endif()

# 设置 Java 命令 (ANTLR 需要 Java)
find_package(Java REQUIRED)
include(UseJava)

option(OPUTOOL_BUILD "Path to the opuisa project root directory" "")
if(NOT OPUTOOL_BUILD)
    # 尝试从环境变量获取
    set(OPUTOOL_BUILD $ENV{OPUTOOL_BUILD})
endif()
if(NOT OPUTOOL_BUILD)
    # 默认相对路径 (假设 opuisa 和 ptxasm 在同一级目录下)
    set(OPUTOOL_BUILD "${CMAKE_CURRENT_SOURCE_DIR}/../build")
endif()


# --- 定位 opuisa 项目和其生成的文件 ---
# 1. 设置 opuisa 项目根目录路径 (相对于当前 CMakeLists.txt)
#    你可以通过 CMake 选项、环境变量或硬编码来设置
option(OPUISA_ROOT "Path to the opuisa project root directory" "")
if(NOT OPUISA_ROOT)
    # 尝试从环境变量获取
    set(OPUISA_ROOT $ENV{OPUISA_ROOT})
endif()
if(NOT OPUISA_ROOT)
    # 默认相对路径 (假设 opuisa 和 ptxasm 在同一级目录下)
    set(OPUISA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../opuisa")
endif()

# 规范化路径并验证
get_filename_component(OPUISA_ROOT "${OPUISA_ROOT}" ABSOLUTE)
if(NOT EXISTS "${OPUISA_ROOT}/coasm_isa.md")
    message(FATAL_ERROR "opuisa directory '${OPUISA_ROOT}' not found or invalid. Please set OPUISA_ROOT correctly.")
endif()
message(STATUS "Found opuisa at: ${OPUISA_ROOT}")

# 2. 定义 opuisa 生成文件的路径
set(OPUISA_GENERATED_DIR "${OPUISA_ROOT}/generated")
set(COASM_GRAMMAR_FILE "${OPUISA_GENERATED_DIR}/coasm.g4")
set(OPUISA_ANTLR_GENERATED_DIR "${OPUISA_GENERATED_DIR}/antlr_generated")

# 3. (可选但推荐) 确保 opuisa 的生成步骤被执行
#    创建一个自定义目标来运行 opuisa 的生成器脚本
#    这假设 opuisa 的 DSL 文件是固定的，或者你有一个机制来触发重新生成
#    如果 opuisa 本身由 CMake 管理，你可以将其作为子工程添加 (见下方替代方法)
add_custom_target(run_opuisa_generator
    COMMAND ${PYTHON_EXECUTABLE} ${OPUISA_ROOT}/scripts/coasm_gen.py -i ${OPUISA_ROOT}/coasm_isa.md -o ${OPUISA_GENERATED_DIR} --antlr
    WORKING_DIRECTORY ${OPUISA_ROOT}
    COMMENT "Ensuring opuisa artifacts (coasm.g4, ANTLR C++) are generated..."
    BYPRODUCTS ${COASM_GRAMMAR_FILE} ${OPUISA_ANTLR_GENERATED_DIR}/*.h ${OPUISA_ANTLR_GENERATED_DIR}/*.cpp
    # DEPENDS ${OPUISA_ROOT}/coasm_isa.md ${OPUISA_ROOT}/scripts/coasm_gen.py # 如果有源文件依赖
)

# 创建一个自定义目标来封装 COASM ANTLR 生成
#add_custom_target(antlr_gen_coasm DEPENDS ${ANTLR_COASM_OUTPUT_FILES})


# --- 定义 ANTLR 生成 PTX 解析器 (保持不变或根据需要调整) ---
set(ANTLR_PTX_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/antlr_generated_ptx)
file(MAKE_DIRECTORY ${ANTLR_PTX_GENERATED_DIR})

set(PTX_GRAMMAR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/grammar)
set(PTX_LEXER_FILE ${PTX_GRAMMAR_DIR}/ptxLexer.g4)
set(PTX_PARSER_FILE ${PTX_GRAMMAR_DIR}/ptxParser.g4)

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


file(GLOB OPUISA_CPP_FILES "${OPUISA_ANTLR_GENERATED_DIR}/*.cpp")
# --- 项目源文件 ---
set(SOURCES
    src/main.cpp
    src/PtxTranslator.cpp
    ${ANTLR_PTX_OUTPUT_FILES}   # 包含 PTX ANTLR 生成的文件
    #${ANTLR_COASM_OUTPUT_FILES} # 包含 COASM ANTLR 生成的文件 # <--- 新增
    # --- 添加 COASM ANTLR 生成的文件 ---
    # 注意：我们不直接列出所有 .cpp 文件，而是将整个目录添加到包含路径
    # 如果需要直接编译 coasm infra 的 .cpp，可以这样做（但通常不推荐，见下文）：
    ${OPUISA_CPP_FILES} # 不推荐直接包含生成的 .cpp
)

# --- 创建可执行文件 ---
add_executable(ptxasm ${SOURCES})

# --- 确保依赖 ---
# 确保在构建 ptxasm 之前运行 opuisa 生成器
add_dependencies(ptxasm run_opuisa_generator)
# 确保在构建 ptxasm 之前先运行 PTX ANTLR 生成
add_dependencies(ptxasm antlr_gen_ptx)

# --- 设置包含目录 ---
target_include_directories(ptxasm PRIVATE
    ${ANTLR_PTX_GENERATED_DIR}   # 包含 PTX ANTLR 生成的头文件
    # --- 包含 COASM ANTLR 生成的头文件目录 ---
    ${OPUISA_ANTLR_GENERATED_DIR} # 包含 coasm infra 生成的 ANTLR 头文件
    ${PTX_GRAMMAR_DIR}     # 包含原始 .g4 文件目录 (如果需要)
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
target_link_directories(ptxasm PRIVATE ${OPUTOOL_BUILD}/lib)
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
