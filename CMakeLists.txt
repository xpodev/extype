cmake_minimum_required(VERSION 3.15...3.25)

project(${SKBUILD_PROJECT_NAME} VERSION ${SKBUILD_PROJECT_VERSION} LANGUAGES CXX)

find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)

Python_add_library(core MODULE WITH_SOABI src/extypes/extypes.cpp)

install(TARGETS core DESTINATION extypes)
