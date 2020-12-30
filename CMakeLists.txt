cmake_minimum_required(VERSION 3.3)

project(compiler VERSION 1.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

find_package(BISON 3.3)
IF(BISON_FOUND)
    BISON_TARGET(grammar parser/grammar.y
                 ${CMAKE_CURRENT_SOURCE_DIR}/parser/grammar.cc
                 DEFINES_FILE ${CMAKE_CURRENT_SOURCE_DIR}/parser/grammar.h)
ENDIF(BISON_FOUND)

find_package(FLEX)
IF(FLEX_FOUND)
    FLEX_TARGET(scanner parser/scanner.l
                ${CMAKE_CURRENT_SOURCE_DIR}/parser/scanner.cc
                DEFINES_FILE ${CMAKE_CURRENT_SOURCE_DIR}/parser/scanner.h)
ENDIF(FLEX_FOUND)

# Compiler binary
add_executable(compiler
    main.cc
    checker/check.cc
    commands/build.cc
    commands/check.cc
    commands/command.cc
    commands/help.cc
    commands/ir.cc
    commands/parse.cc
    commands/run.cc
    core/error.cc
    emitter/emit.cc
    emitter/expression.cc
    emitter/optimize.cc
    parser/ast.cc
    parser/grammar.cc
    parser/parse.cc
    parser/scanner.cc)
target_compile_options(compiler PUBLIC -Wall -Werror -Wno-register)

# UTF-8
target_include_directories(compiler PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ext/utf8/source)

# LLVM
add_subdirectory(ext/llvm/llvm)
target_include_directories(compiler PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ext/llvm/llvm/include)
target_include_directories(compiler PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/ext/llvm/llvm/include)
target_link_libraries(compiler
    LLVMCore
    LLVMipo
    LLVMExecutionEngine
    LLVMInterpreter
    LLVMMCJIT
    LLVMAArch64CodeGen
    LLVMAMDGPUCodeGen
    LLVMARMCodeGen
    LLVMAVRCodeGen
    LLVMBPFCodeGen
    LLVMHexagonCodeGen
    LLVMLanaiCodeGen
    LLVMMipsCodeGen
    LLVMMSP430CodeGen
    LLVMNVPTXCodeGen
    LLVMPowerPCCodeGen
    LLVMRISCVCodeGen
    LLVMSparcCodeGen
    LLVMSystemZCodeGen
    LLVMWebAssemblyCodeGen
    LLVMX86CodeGen
    LLVMXCoreCodeGen)
